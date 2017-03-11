/*
 * EncoderDecoder.cpp
 *
 *  Created on: 12 ����� 2017
 *      Author: Dana Cohen
 */

#include "../include/EncoderDecoder.h"
#include <iostream>
#include "../include/ConnectionHandler.h"

using namespace std;


EncoderDecoder::~EncoderDecoder() {
    delete(dataToDecode);
    delete(opcodeBytes);
    delete(blockNumBytes);
    delete(packetSizeByte);
    delete(blockNumDataBytes);
    delete(data);
}

void EncoderDecoder::cleanFields(){
    dataToDecode->clear();
    opcodeBytes->clear();
    blockNumBytes->clear();
    packetSizeByte->clear();
    blockNumDataBytes->clear();
    startedInitializingOpCode=false;
    isOpCodeInitialized=false;
    doneDecoding=false;
    startedACKBlockNum=false;
    finishedAckBlockNum=false;
    startedPacketSize=false;
    finishedPacketSize=false;
    startedDataBlockNum=false;
    finishedDataBlockNum=false;
    removedOpcode=false;
    readSecond=false;
    opcode=0;
    currPos=0;
    data->clear();
}

void EncoderDecoder::cleanSpecificFields(){
    dataToDecode->clear();
    opcodeBytes->clear();
    blockNumBytes->clear();
    packetSizeByte->clear();
    blockNumDataBytes->clear();
    startedInitializingOpCode=false;
    isOpCodeInitialized=false;
    doneDecoding=false;
    startedACKBlockNum=false;
    finishedAckBlockNum=false;
    startedPacketSize=false;
    finishedPacketSize=false;
    startedDataBlockNum=false;
    finishedDataBlockNum=false;
    removedOpcode=false;
    readSecond=false;
}


ServerPacket* EncoderDecoder:: decodeNextByte(char nextByte, ConnectionHandler* connectionHandler_par) {
    currPos++;
    ServerPacket *packet_6_OR_10=nullptr;
    if (!isOpCodeInitialized) {
        packet_6_OR_10 = initializeOpCode_and_dataToDecode_or_returnRelevantPackets(nextByte);
    }
    if (!removedOpcode && isOpCodeInitialized && readSecond) { // After initialization of the opcode, I pop the two first bytes
        removedOpcode=true;
        popTwoFirstBytes(dataToDecode);
    }

    if (!(packet_6_OR_10==nullptr)) {// It means we need to return DIRQPacket(6) or DISCPacket(10)
        cleanFields();
        currPos=0;
        return packet_6_OR_10;
    }
    else {// When entering this segment, the opcode is initialized and NOT 6 or 10
        //n
        //delete (packet_6_OR_10);
        if (readSecond){
            // Zero at the end:  BCastPacket(9), DELRQPacket(8), LOGRQ(7), ErrorPacket(5), WRQPacket(2), RRQPacket(1)
            if (opcode==9 || opcode ==8 ||  opcode ==7 ||  opcode ==5 ||  opcode ==2 ||  opcode ==1  ) {
                ServerPacket *packet = handleCasesWithZeroAtTheEnd(nextByte); // If the nextByte='\0', the function return the relevant packet
                if (packet != nullptr) {
                    cleanFields();
                    return packet; // return BCastPacket(9), DELRQPacket(8), LOGRQ(7), ErrorPacket(5), WRQPacket(2), RRQPacket(1)
                }
            }

            // DataPacket(3) or AckPacket(4)
            if (opcode==3) {

                DataPacket* d = readDataPacket(nextByte, connectionHandler_par);
                if (d==nullptr) return d;
                else {
                    cleanFields();
                    return d;
                }
                // Why return null ? Because at this cases I have to keep reading ACCORDING the packetSize
            }
            else if (opcode==4) {
                AckPacket* d = readAckPacket(nextByte);
                if (d==nullptr) return d;
                else {
                    cleanSpecificFields();
                    return d;
                }
            }
        }
        // STILL READING
        pushByte(nextByte); // Meaning that i'm still at the reading of the message, (Or at DataPacket case)
        readSecond=(opcodeBytes->size()==2);
        return nullptr;
    }
}

DataPacket* EncoderDecoder::readDataPacket(char nextByte, ConnectionHandler* connectionHandler_par){
    if(startedPacketSize==false){
        startedPacketSize=true;
        packetSizeByte->push_back(nextByte);
        return nullptr;
    }
    else if(finishedPacketSize==false){
        packetSizeByte->push_back(nextByte);
        finishedPacketSize=true;
        packetSize=bytesToShort(*packetSizeByte);
        return nullptr;
    }
    else{
        // At this point, we have the packetSize, and i'm reading the nextByte

        if(startedDataBlockNum==false){
            blockNumDataBytes->push_back(nextByte);
            //blockNumDataBytes.at(0)=nextByte;
            startedDataBlockNum=true;
            return nullptr;
        }
        else if(finishedDataBlockNum==false){
            blockNumDataBytes->push_back(nextByte);
            finishedDataBlockNum=true;
            blockNum=bytesToShort(*blockNumDataBytes);
            leftToRead=packetSize; // Before: leftToRead=packetSize
            if (packetSize == 0 )
            {
                DataPacket* packet=new DataPacket((short)3, packetSize, blockNum, *data);
                return packet;
            }
            return nullptr;
        }
        else { // Enters here only if has the 5th char
            if  (packetSize==512){
                data->push_back(nextByte);
                leftToRead=leftToRead-1;
                if (leftToRead==0){
                    DataPacket* packet=new DataPacket((short)3, packetSize, blockNum, *data);
                    return packet;
                }
            }

            //At this point, we also have the blockNum, and I'm reading the nextByte
            else if(leftToRead>1){
                data->push_back(nextByte);
                leftToRead=leftToRead-1;

                return nullptr;
            }
            else{ // Nothing left to read - FINISHED
                if (leftToRead==1){
                    data->push_back(nextByte);
                    DataPacket* packet=new DataPacket((short)3, packetSize, blockNum, *data);
                    return packet;
                }
            }
        }
    }
    return nullptr;
}

void EncoderDecoder::sendSafeAckPacket(AckPacket* packet, ConnectionHandler* connectionHandler_par){

    short opcode=4;
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




void EncoderDecoder::sendPacket(ServerPacket* packet, ConnectionHandler* connectionHandler_par){
    vector<char>* v = encode(packet);
    sendVecViaCH(v, connectionHandler_par);
}

void EncoderDecoder::sendVecViaCH(vector<char>* dataToSendVec, ConnectionHandler* connectionHandler_par){
    ConnectionHandler* connectionHandler=connectionHandler_par;
    int size = dataToSendVec->size();
    if (size<=512){
        char* dataToSendChar = vecToArr(*dataToSendVec);
        int size=dataToSendVec->size();
        connectionHandler->sendBytes(dataToSendChar, size);
    }
    else  {
        cout << "Can't send more then 512!" << endl;
    }
}

AckPacket* EncoderDecoder::readAckPacket(char nextByte){
    if (startedACKBlockNum==false) {
        blockNumBytes->push_back(nextByte);
        char c = blockNumBytes->at(0);
        //blockNumBytes.at(0)=nextByte;
        startedACKBlockNum=true;
        return nullptr;
    }
    else {
        if (finishedAckBlockNum==false)
            blockNumBytes->push_back(nextByte);
        //blockNumBytes.at(1)=nextByte;
        finishedAckBlockNum=true;
        short blockNum = bytesToShort(*blockNumBytes);
        AckPacket* packet = new AckPacket(opcode, blockNum);
        return packet;
    }
}



// TODO: Problem with the opcode of '10' and '1'
// Checks the 6 cases of BCastPacket(9), ErrorPacket(5), RRQPacket(1), WRQPacket(2), DELRQPacket(8), LOGRQPacket(7)

ServerPacket* EncoderDecoder::handleCasesWithZeroAtTheEnd(char nextByte){


    if (nextByte=='\0') {
        if (opcode==9 && (dataToDecode->size()>1)){ // Meaning we need to return a BCastPacket
            char deleted_or_added=returnFirstByte(*dataToDecode);
            popFirstByte(dataToDecode);
            string s = popString(*dataToDecode);
            BCastPacket* packet = new BCastPacket(opcode,deleted_or_added,s);
            return packet;
        }

        else if (opcode==8){ // Meaning we need to return a DELRQPacket


            string s = popString(*dataToDecode);
            DELRQPacket* packet = new DELRQPacket(opcode,s);
            return packet;
        }
        else if (opcode==7){ // Meaning we need to return a LOGRQPacket


            string s = popString(*dataToDecode);
            LOGRQPacket* packet = new LOGRQPacket(opcode,s);
            return packet;
        }
        else if (opcode==5 && (dataToDecode->size()>1)){ // Meaning we need to return a ErrorPacket

            short errorCode=returnTwoFirstBytes(*dataToDecode);
            popTwoFirstBytes(dataToDecode);
            string s = popString(*dataToDecode);
            ErrorPacket* packet = new ErrorPacket(opcode, errorCode, s);
            return packet;
        }
        else if (opcode==2){ // Meaning we need to return a WRQPacket

            string s = popString(*dataToDecode);
            WRQPacket* packet = new WRQPacket(opcode, s);
            return packet;
        }
        else if (opcode==1){ // Meaning we need to return a RRQPacket
            string s = popString(*dataToDecode);
            RRQPacket* packet = new RRQPacket(opcode, s);
            return packet;
        }
    }
    return nullptr;
}

ServerPacket* EncoderDecoder::initializeOpCode_and_dataToDecode_or_returnRelevantPackets(char nextByte){
    if (!startedInitializingOpCode) {
        opcodeBytes->push_back(nextByte);
        startedInitializingOpCode=true;
        return nullptr;
    }
    else {

        opcodeBytes->push_back(nextByte);
        opcode=bytesToShort(*opcodeBytes);

        isOpCodeInitialized=true;
        if (opcode==6) { DIRQPacket* respond = new DIRQPacket(opcode);
            return (ServerPacket*)respond;
        }
        else if (opcode==10) { DiscPacket* respond = new DiscPacket(opcode);
            return (ServerPacket*)respond;
        }
        else return nullptr;
    }
}


char* EncoderDecoder::vecToArr(vector<char>& v){
    // Get a char pointer to the data in the vector
    char* buf = &v[0]; // For example , vec[1011,.... , 9101], so buf -> 1011 (Later, i'll send it with the size, so it will know when to finish)
    return buf;
}


vector<char> EncoderDecoder::arrToVec(char* c){
    int size=sizeof(c);
    vector<char> v (c, c+size);
    return v;
}



vector<char>* EncoderDecoder::encode(ServerPacket* message){
    CheapManualInitialization();
    short opcode=message->getOpcode();
    char* opcodeCharsArr=new char[2];
    shortToBytes(opcode,opcodeCharsArr);
    char fo = opcodeCharsArr[0];
    char lo = opcodeCharsArr[1];
    opcodeBytes->push_back(fo);
    opcodeBytes->push_back(lo);
    //*opcodeBytes = arrToVec(opcodeCharsArr);
    if(opcode==1){
        vector<char>* response = encodeRRQ(message, *opcodeBytes);
        return response;
    }

    else if (opcode==2){
        vector<char>* response=encodeWRQ(message, *opcodeBytes);
        return response;
    }

    else if (opcode==3){
        vector<char>* response=encodeData(message, *opcodeBytes);
        return response;
    }

    else if (opcode==4){
        vector<char>* response=encodeAck(message, *opcodeBytes);
        return response;
    }

    else if (opcode==5){
        vector<char>* response=encodeError(message, *opcodeBytes);
        return response;
    }

    else if (opcode==6){
        vector<char>* ans = vectorTrim(*opcodeBytes,2); // {0,10}
        return ans;
    }

    else if (opcode==7){
        vector<char>* response=encodeLogRQ(message, *opcodeBytes);
        return response;
    }

    else if (opcode==8){
        vector<char>* response=encodeDelRQ(message, *opcodeBytes);
        return response;
    }

    else if (opcode==9){
        vector<char>* response=encodeBCast(message, *opcodeBytes);
        return response;
    }

    else if (opcode==10){
        vector<char>* ans = vectorTrim(*opcodeBytes,2); // {0,10}
        return ans;
    }

    return nullptr;
}


vector<char>* EncoderDecoder::vectorTrim(vector<char>& myVec, int end) {
    vector<char> *ans = new vector<char>();
    for (unsigned int i = 0; i < end; i++) {
        char tmp = myVec.at(i);
        ans->push_back(tmp);
    }
    return ans;
}

vector<char>* EncoderDecoder::encodeRRQ(ServerPacket* message_par, vector<char>& opcodeBytes){
    string fileName=((RRQPacket*)message_par)->getFileName();
    vector<char> fileNameBytes=getBytes(fileName);
    lenDana=lenDana+fileNameBytes.size()+1;
    vector<char>* response=new vector<char>;
    //vector<char> response=new vector<char>(lenDana);
    for(unsigned int i=0;i<2;i++){
        char c = opcodeBytes.at(i);
        response->push_back(c);
        //response.at(i)=opcodeBytes.at(i);
    }
    for(unsigned int j=0; j<fileNameBytes.size();j++){
        char c = fileNameBytes.at(j);
        response->push_back(c);
        //response.at(j+2)=fileNameBytes.at(j);
    }
    response->push_back(0);
    //response.at(response.size()-1)=0;
    return response;
}


vector<char>* EncoderDecoder:: encodeWRQ(ServerPacket* message, vector<char>& opcodeBytes){

    string fileName=((WRQPacket*)message)->getFileName();
    vector<char> fileNameBytes=getBytes(fileName);
    lenDana=lenDana+fileNameBytes.size()+1;
    vector<char>* response=new vector<char>;
    //vector<char> response=new vector<char>(lenDana);
    for(unsigned int i=0;i<2;i++){
        char c = opcodeBytes.at(i);
        response->push_back(c);
        //response.at(i)=opcodeBytes.at(i);
    }
    for(unsigned int j=0; j<fileNameBytes.size();j++){
        char n = fileNameBytes.at(j);
        response->push_back(n);
        //response.at(j+2)=fileNameBytes.at(j);
    }
    response->push_back(0);
    //response.at(response.size()-1)=0;
    return response;
}
/////////////////////////////////// TOOOOOOOOOOOOOOOOOOODOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO

vector<char>* EncoderDecoder::encodeData(ServerPacket* message, vector<char>& opcodeBytes){
    short packetSize=((DataPacket*)message)->getPacketSize();
//		// Example
//		char* opcodeCharsArr;
//		shortToBytes(opcode,opcodeCharsArr);
//		*opcodeBytes = arrToVec(opcodeCharsArr);
//		// Before
//		vector<char>* packetSizeBytes=shortToBytes(packetSize);
    // Now
    //vector<char>* packetSizeBytes;
    char* packetSizeCharArr=new char[2];

    shortToBytes(packetSize,packetSizeCharArr);
    *packetSizeByte = arrToVec(packetSizeCharArr);
    // End
    short blockNum=((DataPacket*)message)->getBlockNum();
    char* blockNumCharArr=new char[2];
    shortToBytes(blockNum,blockNumCharArr);
    *blockNumBytes = arrToVec(blockNumCharArr);
//		vector<char>* blockNumBytes=shortToBytes(blockNum);
    vector<char> DataBytes=((DataPacket*)message)->getData();
    lenDana=lenDana+packetSizeByte->size()+blockNumBytes->size()+DataBytes.size();
    vector<char>* response=new vector<char>;
    //vector<char> response=new vector<char>(lenDana);
    for(unsigned int i=0;i<2;i++){
        char c = opcodeBytes.at(i);
        response->push_back(c);
        //response.at(i)=opcodeBytes.at(i);
    }
    for(unsigned int j=0; j<2;j++){
        char c2 = packetSizeByte->at(j);
        response->push_back(c2);
        //response.at(j+2)=packetSizeBytes.at(j);
    }
    for(unsigned int k=0;k<2;k++){
        char c3 = blockNumBytes->at(k);
        response->push_back(c3);
        //response.at(k+4)=blockNumBytes.at(k);
    }
    for(unsigned int p=0; p<DataBytes.size();p++){
        char c4 = DataBytes.at(p);
        response->push_back(c4);
        //		response.at(p+6)=DataBytes.at(p);
    }
    return response;
}


vector<char>* EncoderDecoder::encodeAck(ServerPacket* message, vector<char>& opcodeBytes){
//		// Before
//		short blockNum=((AckPacket*)message)->getBlockNum();
//		vector<char>* blockNumBytes=shortToBytes(blockNum);
    // Now
    short blockNum=((AckPacket*)message)->getBlockNum();
    char* blockNumCharArr=new char[2];
    shortToBytes(blockNum,blockNumCharArr);
    *blockNumBytes = arrToVec(blockNumCharArr);
    lenDana=lenDana+2;
    vector<char>* response=new vector<char>;
    //vector<char> response=new vector<char>(lenDana);
    for(unsigned int i=0;i<2;i++){
        char c = opcodeBytes.at(i);
        response->push_back(c);
        //response.at(i)=opcodeBytes.at(i);
    }
    for(unsigned int j=0; j<2;j++){
        char c3 = blockNumBytes->at(j);
        response->push_back(c3);
        //response.at(j+2)=blockNumBytes.at(j);
    }
    return response;
}


vector<char>* EncoderDecoder::encodeError(ServerPacket* message, vector<char>& opcodeBytes){
    // Before
//		short errorCode=((ErrorPacket*)message)->getErrorCode();
//		vector<char>* errorCodeBytes=shortToBytes(errorCode);

    // Now
    short errorCode=((ErrorPacket*)message)->getErrorCode();
    vector<char>* errorCodeBytes;
    char* errorCodeCharArr=new char[2];
    shortToBytes(errorCode,errorCodeCharArr);

    *errorCodeBytes = arrToVec(errorCodeCharArr);
    vector<char> manual;
    for (unsigned int i=0; i<2; i++) {
        char c = errorCodeBytes->at(i);
        manual.push_back(c);
    }
    *errorCodeBytes = manual;

    string errMsg=((ErrorPacket*)message)->getErrMsg();
    vector<char> errMsgBytes=getBytes(errMsg);


    lenDana=opcodeBytes.size()+errorCodeBytes->size()+errMsgBytes.size()+1;

    vector<char>* response=new vector<char>();
    //vector<char> response=new vector<char>(lenDana);
    for(unsigned int i=0;i<2;i++){
        char c = opcodeBytes.at(i);
        response->push_back(c);
        //	response.at(i)=opcodeBytes.at(i);
    }

    for(unsigned int j=0;j<2;j++){
        char c2 = manual.at(j);
        response->push_back(c2);
        //response.at(j+2)=errorCodeBytes.at(j);
    }

    for(unsigned int j=0;j<errMsgBytes.size();j++){
        char c3 = errMsgBytes.at(j);
        response->push_back(c3);
        //response.at(j+4)=errMsgBytes.at(j);
    }
    response->push_back(0);
    //response.at(response.size()-1)=0;
    return response;
}


vector<char>* EncoderDecoder::encodeLogRQ(ServerPacket* message, vector<char>& opcodeBytes){
    string userName=((LOGRQPacket*)message)->getUserName();
    vector<char> userNameBytes=getBytes(userName);
    lenDana=lenDana+userNameBytes.size()+1;
    vector<char>* response=new vector<char>;
    //	vector<char> response=new vector<char>(lenDana);
    for(unsigned int i=0;i<2;i++){
        char c = opcodeBytes.at(i);
        response->push_back(c);
        //response.at(i)=opcodeBytes.at(i);
    }
    for(unsigned int j=0; j<userNameBytes.size();j++){
        char c4 = userNameBytes.at(j);
        response->push_back(c4);
        //response.at(j+2)=userNameBytes.at(j);
    }
    response->push_back(0);
    //response.at(response.size()-1)=0;
    return response;
}


vector<char>* EncoderDecoder::encodeDelRQ(ServerPacket* message, vector<char>& opcodeBytes){
    string fileName=((DELRQPacket*)message)->getFileName();
    vector<char> fileNameBytes=getBytes(fileName);
    lenDana=lenDana+fileNameBytes.size()+1;
    vector<char>* response=new vector<char>;
    //vector<char> response=new vector<char>(lenDana);
    for(unsigned int i=0;i<2;i++){
        char c = opcodeBytes.at(i);
        response->push_back(c);
        //response.at(i)=opcodeBytes.at(i);
    }
    for(unsigned int j=0; j<fileNameBytes.size();j++){
        char c3 = fileNameBytes.at(j);
        response->push_back(c3);
        //response.at(j+2)=fileNameBytes.at(j);
    }
    response->push_back(0);
    //	response.at(response.size()-1)=0;
    return response;
}


vector<char>* EncoderDecoder::encodeBCast(ServerPacket* message, vector<char>& opcodeBytes){
    char delOrAdd=((BCastPacket*)message)->getDelOrAdd();
    string fileName=((BCastPacket*)message)->getFileName();
    vector<char> fileNameBytes=getBytes(fileName);

    lenDana=lenDana+1+fileNameBytes.size()+1;
    vector<char>* response=new vector<char>;
    //vector<char> response=new vector<char>(lenDana);

    for(unsigned int i=0;i<2;i++){
        char c = opcodeBytes.at(i);
        response->push_back(c);
        //response.at(i)=opcodeBytes.at(i);
    }

    response->push_back(delOrAdd);
    //response.at(2)=delOrAdd;

    for(unsigned int j=0;j<fileNameBytes.size();j++){
        char n = fileNameBytes.at(j);
        response->push_back(n);
        //response.at(j+3)=fileNameBytes.at(j);
    }

    response->push_back(0);
    //response.at(response.size()-1)=0;
    return response;
}

EncoderDecoder::EncoderDecoder(){
    dataToDecode=new vector <char>();
    opcodeBytes=new vector <char>();
    blockNumDataBytes=new vector <char>();
    packetSizeByte=new vector <char>();
    blockNumBytes=new vector <char>();
}

void EncoderDecoder::CheapManualInitialization(){
    if (!(dataToDecode== nullptr)){
        delete(dataToDecode);
    }
    dataToDecode=new vector <char>();

    if (!(opcodeBytes== nullptr)){
        delete(opcodeBytes);
    }
    opcodeBytes=new vector <char>();

    if (!(blockNumDataBytes== nullptr)){
        delete(blockNumDataBytes);
    }
    blockNumDataBytes=new vector <char>();

    if (!(packetSizeByte== nullptr)){
        delete(packetSizeByte);
    }
    packetSizeByte=new vector <char>();

    if (!(blockNumBytes== nullptr)){
        delete(blockNumBytes);
    }
    blockNumBytes=new vector <char>();

}

short EncoderDecoder::bytesToShort(vector<char>& bytesArr){
    short result = (short)((bytesArr.at(0) & 0xff) << 8);
    result += (short)(bytesArr.at(1) & 0xff);
    return result;
}

//
//	vector<char>* EncoderDecoder::shortToBytes(short num)
//	{
//		vector<char> bytesArr(2);
//		bytesArr[0]=((num >> 8) & 0xFF) ;
//		bytesArr[1] = (num & 0xFF);
//		vector<char>* vec_ptr = &bytesArr;
//		return vec_ptr;
//	}

void EncoderDecoder::shortToBytes(short num, char* bytesArr)
{
    bytesArr[0] = ((num >> 8) & 0xFF);
    bytesArr[1] = (num & 0xFF);
}




short EncoderDecoder::returnTwoFirstBytes(vector<char> dataToDecode) {
    vector<char> newToDecode;
    //	vector<char> newToDecode= new vector<char>(2);
    for (unsigned int i = 0; i < 2; i++) {
        char c=dataToDecode.at(i);
        newToDecode.push_back(c);
    }
    //	newToDecode.at(i)=dataToDecode.at(i);
    short returnShort=bytesToShort(newToDecode);
    return returnShort;
}


void EncoderDecoder::popTwoFirstBytes(vector<char>* dataToDecode){
    /*
    vector<char> newDataToDecode= new vector<char>(dataToDecode.length-2);
    for(int i=0; i<newDataToDecode.size();i++)
        newDataToDecode.at(i)=dataToDecode.at(i+2);
    this.dataToDecode=newDataToDecode;
    this.len=this.len-2; */
    dataToDecode->erase(dataToDecode->begin());
    dataToDecode->erase(dataToDecode->begin());
}


char EncoderDecoder::returnFirstByte(vector<char>& dataToDecode){
    char firstByte=dataToDecode.at(0);
    return firstByte;
}


void EncoderDecoder::popFirstByte(vector<char>* dataToDecode){
    /*
    vector<char> newDataToDecode= new vector<char>(dataToDecode.length-1);
    for(int i=0; i<newDataToDecode.size();i++)
        newDataToDecode.at(i)=dataToDecode.at(i+1);
    this.dataToDecode=newDataToDecode; */
    dataToDecode->erase(dataToDecode->begin());
}

//	template<typename ServerPacket >
//	void EncoderDecoder<ServerPacket>::popLastByte(vector<char> dataToDecode){
//		/*
//		vector<char> newDataToDecode= new byte[dataToDecode.size()-1];
//		for(int i=1; i<newDataToDecode.size();i++)
//			newDataToDecode.at(i)=dataToDecode.at(i);
//		this.dataToDecode=newDataToDecode; */
//		dataToDecode.pop_back();
//	}


string EncoderDecoder::popString(vector<char>& bytes) {
    //notice that we explicitly requesting that the string will be decoded from UTF-8
    //this is not actually required as it is the default encoding in java.

    /*
    int len=bytes.length;
    string result = new string(bytes, 0, len, StandardCharsets.UTF_8);
    len = 0;
    return result;
    */

    std::string s(bytes.begin(), bytes.end());
    return s;

}


vector<char> EncoderDecoder::getBytes(string s){
    string str=s;
    std::vector<char> data(str.begin(), str.end());
    return data;
}

	vector<char>* EncoderDecoder::vectorTrim(vector<char>& myVec, int start, int end){
        vector<char> newVec(start, end);
        vector<char>* vec_ptr = new vector<char> (newVec);
        return vec_ptr;
	}


// Not for DataPacket
void EncoderDecoder::pushByte(char nextByte) {
    dataToDecode->push_back(nextByte);
}



