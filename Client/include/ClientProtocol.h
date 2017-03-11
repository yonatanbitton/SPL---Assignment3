/*
 * ClientProtocol.h
 *
 *  Created on: Jan 13, 2017
 *      Author:
 */
#ifndef SRC_CLIENTPROTOCOL_H_
#define SRC_CLIENTPROTOCOL_H_
#include <string>
#include <vector>
using namespace std;
#include "../include/ServerPacket.h"
#include "../include/EncoderDecoder.h"
#include "../include/ConnectionHandler.h"
class ConnectionHandler;
class ClientProtocol {

private:
    // FIELDS
    string readReq="def";
    string uploadReq="def";
    string logReq="def";
    string discReq="def";
    string delReq="def";
    string dirqReq="def";
    string lastOperation="def";
    EncoderDecoder encdec;
    short numOfChunksSent=0;
    vector<char>* toSendVec;
    bool goingToFinish=false;
    vector<char>* wholeFile = new vector<char>();


public:
    ClientProtocol();
    virtual ~ClientProtocol();
    //  ClientProtocol();
    ServerPacket* processFromKeyboard(string line);
    void process(ServerPacket* packet, ConnectionHandler* connectionHandler);
    void sendDataAttachments(short size,ConnectionHandler* connectionHandler_par);
    void sendDataAttachments(ConnectionHandler* connectionHandler_par);
    void shortToBytes(short num, char* bytesArr);
    vector <char>* takeFirst512();
//    vector<vector<char>*>* wholeFile = new vector<vector<char>*>();
    // METHODS
    vector <char>* stealFromSendVec();
    ServerPacket* HandleWithRequest(string type, string what);
    void dealWithERROR(ServerPacket* packet, ConnectionHandler* connectionHandler_par);
    void dealWithACK(ServerPacket* packet, ConnectionHandler* connectionHandler);
    void dealWithBCAST(ServerPacket* packet);
    void dealWithDATA(ServerPacket* packet, ConnectionHandler* connectionHandler);
    void dealWithRRQ(ServerPacket* packet, ConnectionHandler* connectionHandler);
    char* vecToArr(vector<char>& v);
    vector<char> arrToVec(char* c);
    void sendVecViaCH(vector<char>* dataToSendVec, ConnectionHandler* connectionHandler_par);
    void sendPacket(ServerPacket* packet, ConnectionHandler* connectionHandler_par);
    bool sendNextPacket(ConnectionHandler* connectionHandler_par);
    vector <char> takeFirstK(int K, vector<char>* toTakeFrom);
    void printDirq(vector<char> &v);
    void printChar(const char c) const;
    void printBytes(const char bytes[], int size) const;
    void sendSafeAckPacket(AckPacket* packet, ConnectionHandler* connectionHandler_par);

    };

#endif /* SRC_CLIENTPROTOCOL_H_ */