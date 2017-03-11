/*
 * ServerPacket.h
 *
 *  Created on: 12 ����� 2017
 *      Author: Dana Cohen
 */

#ifndef SRC_SERVERPACKET_H_
#define SRC_SERVERPACKET_H_
#include <vector>
#include <string>
using namespace std;


class ServerPacket {
public:
	ServerPacket(short opcode_par);
	ServerPacket(const ServerPacket& otherServerPacket);
	virtual ~ServerPacket();
	short getOpcode() const;

private:
	short opcode;
};



class RRQPacket: public ServerPacket {
public:
	RRQPacket(short opcode_par, string fileName_par);
	RRQPacket(const RRQPacket& otherRRQPacket);
	string getFileName() const;
	virtual ~RRQPacket();

private:
	string fileName;
};





class WRQPacket: public ServerPacket {
public:
	WRQPacket(short opcode_par, string fileName_par);
	WRQPacket(const WRQPacket& otherwRQPacket);
	string getFileName() const;
	virtual ~WRQPacket();

private:
	string fileName;
};




class DataPacket: public ServerPacket {
public:
	DataPacket(short opcode_par, short packetSize_par, short blockNum_par, vector<char> data_par);
	DataPacket(const DataPacket& otherDataPacket);
	short getPacketSize() const;
	short getBlockNum() const;
	vector<char> getData() const;
	virtual ~DataPacket();

private:
	short packetSize;
	short blockNum;
	vector<char> data;
};





class AckPacket: public ServerPacket {
public:
	AckPacket(short opcode_par, short blockNum_par) ;
	AckPacket(const AckPacket& otherAckPacket);
	short getBlockNum()const;
	virtual ~AckPacket();

private:
	short blockNum;
};





class ErrorPacket: public ServerPacket {
public:
	ErrorPacket(short opcode_par, short ErrCode_par, string ErrMsg_par);
	ErrorPacket(const ErrorPacket& otherErrorPacket);
	short getErrorCode() const;
	string getErrMsg() const;
	virtual ~ErrorPacket();

private:
	short ErrCode;
	string ErrMsg;
};





class DIRQPacket: public ServerPacket {
public:
	DIRQPacket(short opcode_par);
	DIRQPacket(const DIRQPacket& otherDIRQPacket);
	virtual ~DIRQPacket();

};





class LOGRQPacket: public ServerPacket {
public:
	LOGRQPacket(short opcode_par, string userName_par);
	LOGRQPacket(const LOGRQPacket& otherLOGRQPacket);
	string getUserName() const;
	virtual ~LOGRQPacket();

private:
	string userName;
};





class DELRQPacket: public ServerPacket {
public:
	DELRQPacket(short opcode_par, string fileName_par);
	DELRQPacket(const DELRQPacket& otherDELRQPacket);
	string getFileName() const;
	virtual ~DELRQPacket();

private:
	string fileName;
};




class BCastPacket: public ServerPacket {
public:
	BCastPacket(short opcode_par, char delOrAdd_par, string fileName_par);
	BCastPacket(const BCastPacket& otherBCastPacket);
	char getDelOrAdd() const;
	string getFileName() const;
	virtual ~BCastPacket();

private:
	char delOrAdd;
	string fileName;
};





class DiscPacket: public ServerPacket {
public:
	DiscPacket(short opcode_par);
	DiscPacket(const DiscPacket& otherDiscPacket);
	virtual ~DiscPacket();

};


#endif /* SRC_SERVERPACKET_H_ */
