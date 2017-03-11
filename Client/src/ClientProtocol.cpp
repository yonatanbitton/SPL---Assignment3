//
// Created by bittoy on 1/14/17.
//
#include "../include/ClientProtocol.h"
#include <iostream>
#include <fstream>


using namespace std;
#include <algorithm>
//#include <boost/filesystem.hpp>
//using namespace boost::filesystem;
/*
 * ClientProtocol.cpp
 *
 *  Created on: Jan 13, 2017
 *      Author: Yonatan Bitton
 */
class connectionHandler;


ClientProtocol::ClientProtocol():encdec(),toSendVec(){}

void ClientProtocol::process(ServerPacket* packet, ConnectionHandler* _connectionHandler_par){
    ConnectionHandler* connectionHandler=_connectionHandler_par;
    short opcode=packet->getOpcode();
    bool unfamiliar=false;
//    try {
//        if (opcode != 4 || opcode != 9 || opcode != 3 || opcode != 1 || opcode != 5)
//            throw "Unfamiliar opcode on process";
//    }   catch (string msg) {cout << msg << endl;}
    if ( (!(opcode==4)) && (!(opcode==9)) && (!(opcode==3)) && (!(opcode==1)) && (!(opcode==5)) ) {
//        cout << "Unfamiliar opcode on process" << endl;
        unfamiliar=true;
    }
    if (unfamiliar==false) {
        if (opcode == 4) {
            dealWithACK(packet, connectionHandler);
        } else if (opcode == 9) {
            dealWithBCAST(packet);
        } else if (opcode == 3) {
            dealWithDATA(packet, connectionHandler);
        }
        else if (opcode == 5) {
            dealWithERROR(packet, connectionHandler);
        }
    }
}

void ClientProtocol::dealWithACK(ServerPacket* packet, ConnectionHandler* connectionHandler_par){
    if (!(ClientProtocol::logReq.compare("def")==0)){
        cout << "ACK 0" << endl;
        logReq= "def";
    }
    else if (!(ClientProtocol::discReq.compare("def")==0)){
        cout << "ACK 0" << endl;
        discReq= "def";
        connectionHandler_par->setConnectionState(false);
        connectionHandler_par->close();
    }
    else if (!(ClientProtocol::delReq.compare("def")==0)){
        cout << "ACK 0" << endl;
        delReq= "def";
    }
        // If I wanted to upload something, and I got a recieve from the server that I may.
    else if (!(ClientProtocol::uploadReq.compare("def")==0)) {
        AckPacket *myPacket = dynamic_cast<AckPacket*>(packet);
        if (goingToFinish && toSendVec != nullptr && toSendVec->size()==0) {
            short blockNum = myPacket->getBlockNum();
            cout << "ACK " << blockNum << endl;
            cout << "WRQ " << uploadReq << " complete" << endl;
            goingToFinish=false;
            uploadReq="def";
        } else {
            short blockNum = myPacket->getBlockNum();
            if (blockNum == 0) {
                cout << "ACK 0" << endl;
                string path = uploadReq;
                std::ifstream input(path, std::ios::binary); // Reading from the file
                toSendVec = new std::vector<char>((std::istreambuf_iterator<char>(input)),
                                                  (std::istreambuf_iterator<char>()));  // On the first ack, (ACK 0), we initialize the field "toSendVec", and then we will keep sending from there.

                if (toSendVec->size()>0) {
                    if (sendNextPacket(connectionHandler_par)) {  // Sending the first packet of the data

                        goingToFinish = true;
                    }
                } else {
//                    cout << "Cant find file " << endl;
                }
            } else { // the blockNum is 1,..2,...3...
                cout << "ACK " << blockNum << endl;
                if (sendNextPacket(connectionHandler_par)) {
                    goingToFinish = true;
                }// This function sends the next 512 bytes, and returns true if finished
            }
//        delete (myPacket);

        }
    }
}


// This function sends packet to the client. If the toSendVec.size < 512 (No more to send) returns true;
// Else if the toSendVec.size >= 512 - takes the first 512 bytes, and sends it.
bool ClientProtocol::sendNextPacket(ConnectionHandler* connectionHandler_par) {
    if (toSendVec->size() < 512) {// If the rest of what left to send is < 512 , so we need to send it, and return true - finished uploading.
        vector<char>* stolen = stealFromSendVec(); // Takes all of the data from toSendVec to stolen (Removes it)
        short localPacketSize=stolen->size();
        sendDataAttachments(localPacketSize, connectionHandler_par);
        sendVecViaCH(stolen, connectionHandler_par);
        return true;
    }
    else { // If the rest of what left to send is < 512 , so we need to send it, and return true - finished uploading.
        vector<char>* prefix = takeFirst512(); // Takes the first 512 from the toSendVec (Removes it)
        sendDataAttachments((short)512, connectionHandler_par);
        sendVecViaCH(prefix, connectionHandler_par);
        return false;
    }
}


// Sends the opcodeBytes, packetSizeBytes, blockNumBytes
void ClientProtocol::sendDataAttachments(short localPacketSize, ConnectionHandler* connectionHandler_par){
   // generalSize = toSendVec->size();
    char* opcodeBytes = new char[2];
    shortToBytes(3, opcodeBytes);
//    char firstOpcodeByte = toSendVec->at(0);
//    char secondOpcodeByte = toSendVec->at(1);
//    short packetSize=-1;
//    if (generalSize>=512) {
//        generalSize=generalSize-512;
//        packetSize=512;
//    }
//    else // generalsize < 512
//        packetSize=generalSize;
    char* packetSizeBytes = new char[2];
    shortToBytes(localPacketSize, packetSizeBytes);
//    char firstPacketsizeBytes =
    numOfChunksSent++;
    char* blockNumBytes = new char[2];
    shortToBytes(numOfChunksSent,blockNumBytes);
//    cout << "The blockNumBytes=" << bytesToShort(blockNumBytes) << endl;
    vector<char>* attachment = new vector<char>();
    for (unsigned int i=0; i<2; i++) {
        attachment->push_back(opcodeBytes[i]);
    }
    for (unsigned int i=0; i<2; i++) {
        attachment->push_back(packetSizeBytes[i]);
    }
    for (unsigned int i=0; i<2; i++) {
        attachment->push_back(blockNumBytes[i]);
    }
    sendVecViaCH(attachment, connectionHandler_par);
}



void ClientProtocol::sendPacket(ServerPacket* packet, ConnectionHandler* connectionHandler_par){
    vector<char>* v = encdec.encode(packet);
    sendVecViaCH(v, connectionHandler_par);
}

void ClientProtocol::sendVecViaCH(vector<char>* dataToSendVec, ConnectionHandler* connectionHandler_par){
    ConnectionHandler* connectionHandler=connectionHandler_par;
    int size = dataToSendVec->size();
    if (size<=512){
        char* dataToSendChar = vecToArr(*dataToSendVec);
        int size=dataToSendVec->size();
        connectionHandler->sendBytes(dataToSendChar, size);
    }
    else  {
//        cout << "Can't send more then 512!" << endl;
    }
}

vector <char>* ClientProtocol::stealFromSendVec(){
    vector <char>* stolen=new vector<char>();
    for (unsigned int i=0; i<toSendVec->size(); i++) {
        char c = toSendVec->at(i);
        stolen->push_back(c);
    }
    while (toSendVec->size()>0)
        toSendVec->pop_back();
    return stolen;
}

vector <char>* ClientProtocol::takeFirst512(){
    vector <char>* prefix=new vector<char>();
    vector <char>* yeter=new vector<char>();
    for (unsigned int i=0; i<512; i++) {
        char c = toSendVec->at(i);
        prefix->push_back(c);
    }
    for (unsigned int j=0; j<toSendVec->size()-512; j++) {
        char d = toSendVec->at(j+512);
        yeter->push_back(d);
    }
    toSendVec=yeter;
    return prefix;
}



void ClientProtocol::shortToBytes(short num, char* bytesArr)
{
    bytesArr[0] = ((num >> 8) & 0xFF);
    bytesArr[1] = (num & 0xFF);
}
//
//// This function takes 1 chunk, sends it, and returns true if no chunks are left
//bool ClientProtocol::sendNextPacket(ConnectionHandler* connectionHandler_par) {
//    vector<char> *v = FileToUpload->at(numOfChunksSent);
//    sendVecViaCH(v, connectionHandler_par);
//    numOfChunksSent++;
//    if (numOfChunksSent==FileToUpload->size()) return true;
//    else return false;
//
//}


void ClientProtocol::dealWithBCAST(ServerPacket* packet){ // AS
    BCastPacket* myPacket =static_cast<BCastPacket*>(packet);
//    if (myPacket->getFileName().compare("def")==0 ) {
//        throw (2);
//    }
    string fileName = myPacket->getFileName();
    char add_or_del = myPacket->getDelOrAdd();
    string add_or_del_str;
    if (add_or_del == 0)  add_or_del_str="del";
        else if (add_or_del== 1) add_or_del_str="add";
    cout << "BCAST " <<  add_or_del_str << " " << fileName << endl;
   // delete (myPacket);
}
void ClientProtocol::dealWithDATA(ServerPacket* packet, ConnectionHandler* connectionHandler_par) { // readReq or dirqReq
    DataPacket *myPacket = static_cast<DataPacket *>(packet);
    short blockNum = myPacket->getBlockNum();
    vector<char> data = myPacket->getData();
    short packetSize = myPacket->getPacketSize();
    //  delete(myPacket);
    if (packetSize > 512) {
        ErrorPacket *err = new ErrorPacket((short) 5, (short) 0, "dealWithData:: I've got a packet bigger then 512");
        sendPacket(err, connectionHandler_par);
        //   delete (err);
    }
    if (ClientProtocol::lastOperation.compare("dirqReq") == 0) {
        if (packetSize == 0) {
            AckPacket *ackPacket = new AckPacket((short) 4, blockNum);
            sendPacket(ackPacket, connectionHandler_par);
            printDirq(*wholeFile);
            dirqReq = "def";
        } else if (packetSize == 512) { // the names of the files are larger then 512 bytes
            for (unsigned int i = 0; i < data.size(); i++) {
                char c = data.at(i);
                wholeFile->push_back(c);
            }
            AckPacket *ackPacket = new AckPacket((short) 4, blockNum);
            sendPacket(ackPacket, connectionHandler_par);
        } else if (packetSize > 0 && packetSize < 512) { // Stopping condition (Including the case of <512 bytes
            for (unsigned int i = 0; i < data.size(); i++) {
                char c = data.at(i);
                wholeFile->push_back(c);
            }
            AckPacket *ackPacket = new AckPacket((short) 4, blockNum);
            sendPacket(ackPacket, connectionHandler_par);
            printDirq(*wholeFile);
            dirqReq = "def";
            wholeFile->clear();
        } else {
            cout << "The DIRQ from the server is empty" << endl;
        }
    } else if (ClientProtocol::lastOperation.compare("readReq") == 0) {
        if (packetSize == 512) {
            for (unsigned int i = 0; i < data.size(); i++) {
                char c = data.at(i);
                wholeFile->push_back(c);
            }
            AckPacket *ackPacket = new AckPacket((short) 4, blockNum);
            sendSafeAckPacket(ackPacket, connectionHandler_par);
        } else if (packetSize < 512) {  // Knows I need to assemble the file.
            for (unsigned int i = 0; i < data.size(); i++) {
                char c = data.at(i);
                wholeFile->push_back(c);
            }
            AckPacket *ackPacket = new AckPacket((short) 4, blockNum);
            sendSafeAckPacket(ackPacket, connectionHandler_par);
            string path = readReq;
            char *toWrite = vecToArr(*wholeFile);
            ofstream streamer;
            std::ofstream output( path, std::ios::binary ); // Create a new binary file and write to it.
            output.write(toWrite, wholeFile->size());
            output.close();
            cout << "RRQ " << ClientProtocol::readReq << " complete" << endl;
            readReq = "def";
            wholeFile->clear();
        }
    }
}


void ClientProtocol::sendSafeAckPacket(AckPacket* packet, ConnectionHandler* connectionHandler_par){
    char* opcodeBytes = new char[2];
    shortToBytes(4, opcodeBytes);

    short blockNum = packet->getBlockNum();
    char* blockNumBytes = new char[2];
    shortToBytes(blockNum,blockNumBytes);

    vector<char>* toSend = new vector<char>();
    for (unsigned int i=0; i<2; i++) {
        toSend->push_back(opcodeBytes[i]);
    }
    for (unsigned int i=0; i<2; i++) {
        toSend->push_back(blockNumBytes[i]);
    }
    sendVecViaCH(toSend, connectionHandler_par);
}


//    string ClientProtocol::popString(vector<char> &bytes) {
//
//        std::string s(bytes.begin(), bytes.end());
//        return s;
//
//    }


    void ClientProtocol::printDirq(vector<char> &v) {
        string ans_str = "";
        for (unsigned int i = 0; i < v.size(); i++) {
            ans_str += v.at(i);
        }
        std::replace(ans_str.begin(), ans_str.end(), '\0', '\n');
        cout << ans_str << endl;
        v.clear();
    }

    void ClientProtocol::dealWithERROR(ServerPacket *packet, ConnectionHandler *connectionHandler_par) {
        ErrorPacket *myPacket = static_cast<ErrorPacket *>(packet);
        short errorCode = myPacket->getErrorCode();
        //  delete (myPacket);
        cout << "Error " << errorCode << endl;
        if (lastOperation.compare("logReq") == 0) ClientProtocol::logReq = "def";
        else if (ClientProtocol::lastOperation.compare("discReq") == 0) ClientProtocol::discReq = "def";
        else if (ClientProtocol::lastOperation.compare("delReq") == 0) ClientProtocol::delReq = "def";
        else if (ClientProtocol::lastOperation.compare("uploadReq") == 0) ClientProtocol::uploadReq = "def";
        else if (ClientProtocol::lastOperation.compare("readReq") == 0) ClientProtocol::readReq = "def";
        else if (ClientProtocol::lastOperation.compare("dirqReq") == 0) ClientProtocol::dirqReq = "def";
        else {
            ErrorPacket *err = new ErrorPacket((short) 5, (short) 0,
                                               "The data received is not because of file uploading nor dirq request :O");
            sendPacket(err, connectionHandler_par);
            // delete (err);
        }

    }

    ServerPacket *ClientProtocol::processFromKeyboard(string line) {
        ServerPacket *packet = nullptr;
        vector<string> data_str;

        // trim it from the start
        while (line.at(0) == ' ') {
            line = line.substr(1);
        }
        // trim it from the end
        while (line.at(line.length() - 1) == ' ') {
            line = line.substr(0, line.length() - 1);
        }

        string RequestType = "";
        string RequestWhat = "";

        // "RRQ     COMMENT"
        unsigned int index = 0;
        while (index < line.length()) {
            while (line.at(index) == ' ') index++;
            RequestType = RequestType + line.at(index);
            if ((index + 1) < line.length()) index++;

            while (line.at(index) != ' ') {
                RequestType = RequestType + line.at(index);
                if ((index + 1) < line.length()) index++;
                else if (RequestType.length() >= line.length()) break;
            }
            if (index == line.length() - 1) break;
            while (line.at(index) == ' ') index++;
            if (line.length() - RequestType.length() > 2) RequestWhat = RequestWhat +
                                                                        line.at(index); // I want it to happen, if the sifa > 1
            if ((index + 1) < line.length()) index++;
            while (index < line.length()) {
                RequestWhat = RequestWhat + line.at(index);
                index = index + 1;
                if ((index == line.length())) break;
            }
            break;
        }

        if (!(line.length() > index) || RequestWhat.compare("") == 0) {
            if (RequestType == "DIRQ") {
                dirqReq = RequestWhat;
                lastOperation = "dirqReq";
                packet = new DIRQPacket(6);
                return packet;
            } else if (RequestType == "DISC") {
                discReq = RequestWhat;
                lastOperation = "discReq";
                packet = new DiscPacket(10);
                return packet;
            } else {
                if (!(RequestWhat.compare("") == 0)) {
                    if (RequestType.compare("RRQ") == 0 || RequestType.compare("WRQ") == 0 ||
                        RequestType.compare("LOGRQ") == 0 || RequestType.compare("DELRQ") == 0) {
                        packet = HandleWithRequest(RequestType, RequestWhat);
                        return packet;
                    }
                } else {
                    return nullptr;
                }
            }
        }
        return nullptr;
    }

    ServerPacket *ClientProtocol::HandleWithRequest(string type, string what) {

        ServerPacket *packet;
        if (type == "RRQ") {
            readReq = what;
            lastOperation = "readReq";
            packet = new RRQPacket(1, what);
            wholeFile = new vector<char>();
        } else if (type == "WRQ") {
            uploadReq = what;
            lastOperation = "uploadReq";
            packet = new WRQPacket(2, what);
        } else if (type == "LOGRQ") {
            logReq = what;
            lastOperation = "logReq";
            packet = new LOGRQPacket(7, what);
        } else if (type == "DELRQ") {
            delReq = what;
            lastOperation = "delReq";
            packet = new DELRQPacket(8, what);
        }
        return packet;
    }


    char *ClientProtocol::vecToArr(vector<char> &v) {
        char *buf = &v[0]; // For example , vec[1011,.... , 9101], so buf -> 1011 (Later, i'll send it with the size, so it will know when to finish)
        return buf;
    }

    vector<char> ClientProtocol::arrToVec(char *c) {
        int size = sizeof(c);
        vector<char> v(c, c + size);
        return v;
    }


    ClientProtocol::~ClientProtocol() {
        delete(wholeFile);
    }


