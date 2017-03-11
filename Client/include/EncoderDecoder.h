/*
 * EncoderDecoder.h
 *
 *  Created on: 12 ����� 2017
 *      Author: Dana Cohen
 */
#pragma once
#ifndef SRC_ENCODERDECODER_H_
#define SRC_ENCODERDECODER_H_
#include <vector>
#include <string>
using namespace std;
#include "../include/ServerPacket.h"
class ConnectionHandler;
//template<typename ServerPacket >
class EncoderDecoder {
public:
	virtual ~EncoderDecoder();
	EncoderDecoder();
    ServerPacket* decodeNextByte(char nextByte, ConnectionHandler* connectionHandler_par);
    vector<char>* encode(ServerPacket* message);
	void cleanFields();
	vector<char> getBytes(string s);
	void cleanSpecificFields();
    vector<char>* encodeRRQ(ServerPacket* message, vector<char>& opcodeBytes);
    vector<char>* encodeWRQ(ServerPacket* message, vector<char>& opcodeBytes);
    vector<char>* encodeData(ServerPacket* message, vector<char>& opcodeBytes);
    vector<char>* encodeAck(ServerPacket* message, vector<char>& opcodeBytes);
    vector<char>* encodeError(ServerPacket* message, vector<char>& opcodeBytes);
    vector<char>* encodeLogRQ(ServerPacket* message, vector<char>& opcodeBytes);
    vector<char>* encodeDelRQ(ServerPacket* message, vector<char>& opcodeBytes);
    vector<char>* encodeBCast(ServerPacket* message, vector<char>& opcodeBytes);
    short returnTwoFirstBytes(vector<char> dataToDecode);
    void popTwoFirstBytes(vector<char>* dataToDecode);
    char returnFirstByte(vector<char>& dataToDecode);
    void popFirstByte(vector<char>* dataToDecode);
    ServerPacket* initializeOpCode_and_dataToDecode_or_returnRelevantPackets(char nextByte);
    DataPacket* readDataPacket(char nextByte, ConnectionHandler* connectionHandler);
    AckPacket* readAckPacket(char nextByte);
    ServerPacket* handleCasesWithZeroAtTheEnd(char nextByte);
    void sendSafeAckPacket(AckPacket* packet, ConnectionHandler* connectionHandler_par);
    void sendVecViaCH(vector<char>* dataToSendVec, ConnectionHandler* connectionHandler_par);
    string popString(vector<char>& bytes);
    void pushByte(char nextByte);
    short bytesToShort(vector<char>& bytesArr);
    void shortToBytes(short num, char* bytesArr);
    vector<char>* vectorTrim(vector<char>& myVec, int end);
    void CheapManualInitialization();
    vector<char> arrToVec(char* c);
    void sendPacket(ServerPacket* packet, ConnectionHandler* connectionHandler_par);
    char* vecToArr(vector<char>& v);
    vector<char>* vectorTrim(vector<char>& myVec, int start, int end);



        private:
	short opcode=0;
	int len=0;
	int lenDana=2;
    bool startedInitializingOpCode=false;
    bool isOpCodeInitialized=false;
    bool doneDecoding=false;
    bool startedACKBlockNum=false;
	bool finishedAckBlockNum=false;
	bool startedPacketSize=false;
	bool finishedPacketSize=false;
	bool startedDataBlockNum=false;
	bool finishedDataBlockNum=false;
    bool removedOpcode=false;
    bool readSecond=false;
    vector<char> *dataToDecode;
    vector<char> *opcodeBytes;
    vector<char> *blockNumDataBytes;
    vector<char> *packetSizeByte;
    vector<char> *blockNumBytes;
    short packetSize=0;
	short blockNum=0;
	int leftToRead;
	vector<char> *data=new vector<char>();
	int currPos=0;


    };




#endif /* SRC_ENCODERDECODER_H_ */

