/*
 * ServerPacket.cpp
 *
 *  Created on: 12 ����� 2017
 *      Author: Dana Cohen
 */

#include "../include/ServerPacket.h"
#include <iostream>


ServerPacket::ServerPacket (short opcode_par): opcode(opcode_par) {}

ServerPacket::ServerPacket(const ServerPacket& otherServerPacket): opcode(otherServerPacket.getOpcode()){

}

short ServerPacket:: getOpcode() const{
	return opcode;
}

ServerPacket::~ServerPacket() {
	// TODO Auto-generated destructor stub
}





RRQPacket::RRQPacket(short opcode_par, string fileName_par): ServerPacket(opcode_par),fileName(fileName_par){}
RRQPacket::RRQPacket(const RRQPacket &otherRRQPacket): ServerPacket(1), fileName(otherRRQPacket.getFileName()) {}
string RRQPacket::getFileName() const{
	return fileName;
}
RRQPacket::~RRQPacket(){}





WRQPacket::WRQPacket(short opcode_par, string fileName_par): ServerPacket(opcode_par),fileName(fileName_par){}
WRQPacket::WRQPacket(const WRQPacket& otherwRQPacket): ServerPacket(2), fileName(otherwRQPacket.getFileName()){}
string WRQPacket::getFileName() const{
	return fileName;
}
WRQPacket::~WRQPacket(){}





DataPacket::DataPacket(short opcode_par, short packetSize_par, short blockNum_par, vector<char> data_par): ServerPacket(opcode_par), packetSize(packetSize_par), blockNum(blockNum_par), data(data_par){}
DataPacket::DataPacket(const DataPacket& otherDataPacket): ServerPacket(3), packetSize(otherDataPacket.getPacketSize()), blockNum(otherDataPacket.getBlockNum()),data(otherDataPacket.getData()) {}
short DataPacket::getPacketSize() const{
	return packetSize;
}
short DataPacket::getBlockNum() const{
	return blockNum;
}
vector<char> DataPacket::getData() const{
	return data;
}
DataPacket::~DataPacket(){}





AckPacket::AckPacket(short opcode_par, short blockNum_par): ServerPacket(opcode_par), blockNum(blockNum_par){}
AckPacket::AckPacket(const AckPacket &otherAckPacket) : ServerPacket(4), blockNum(otherAckPacket.getBlockNum()){}
short AckPacket::getBlockNum() const{
	return blockNum;
}
AckPacket::~AckPacket(){}





ErrorPacket::ErrorPacket(short opcode_par, short ErrCode_par, string ErrMsg_par): ServerPacket(opcode_par), ErrCode(ErrCode_par), ErrMsg(ErrMsg_par){}
ErrorPacket::ErrorPacket(const ErrorPacket& otherErrorPacket): ServerPacket(5), ErrCode(otherErrorPacket.getErrorCode()), ErrMsg(otherErrorPacket.getErrMsg()){}
short ErrorPacket::getErrorCode() const{
	return ErrCode;
}
string ErrorPacket::getErrMsg() const{
	return ErrMsg;
}
ErrorPacket::~ErrorPacket(){}





DIRQPacket::DIRQPacket(short opcode_par): ServerPacket(opcode_par){}
DIRQPacket::DIRQPacket(const DIRQPacket& otherDIRQPacket): ServerPacket(6){}
DIRQPacket::~DIRQPacket(){}





LOGRQPacket::LOGRQPacket(short opcode_par, string userName_par): ServerPacket(opcode_par), userName(userName_par){}
LOGRQPacket::LOGRQPacket(const LOGRQPacket& otherLOGRQPacket): ServerPacket(7), userName(otherLOGRQPacket.getUserName()){}
string LOGRQPacket::getUserName() const{
	return userName;
}
LOGRQPacket::~LOGRQPacket(){}





DELRQPacket::DELRQPacket(short opcode_par, string fileName_par): ServerPacket(opcode_par), fileName(fileName_par){}
DELRQPacket::DELRQPacket(const DELRQPacket& otherDELRQPacket):ServerPacket(8), fileName(otherDELRQPacket.getFileName()){}
string DELRQPacket::getFileName() const{
	return fileName;
}
DELRQPacket::~DELRQPacket(){}





BCastPacket::BCastPacket(short opcode_par, char delOrAdd_par, string fileName_par): ServerPacket(opcode_par), delOrAdd(delOrAdd_par), fileName(fileName_par){}
BCastPacket::BCastPacket(const BCastPacket& otherBCastPacket):ServerPacket(9), delOrAdd(otherBCastPacket.getDelOrAdd()), fileName(otherBCastPacket.getFileName()){}

char BCastPacket::getDelOrAdd() const{
	return delOrAdd;
}
string BCastPacket::getFileName() const{
	return fileName;
}
BCastPacket::~BCastPacket(){}





DiscPacket::DiscPacket(short opcode_par): ServerPacket(opcode_par){}
DiscPacket::DiscPacket(const DiscPacket& otherDiscPacket):ServerPacket(10){}
DiscPacket::~DiscPacket(){}
