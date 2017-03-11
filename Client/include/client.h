/*
 * client.h
 *
 *  Created on: Jan 13, 2017
 *      Author: bittoy
 */
#pragma once
#ifndef BOOST_ECHO_CLIENT_SRC_CLIENT_H_
#define BOOST_ECHO_CLIENT_SRC_CLIENT_H_
#include <vector>
#include "ConnectionHandler.h"
#include "../include/EncoderDecoder.h"
#include "../include/ClientProtocol.h"
using namespace std;


class client {
public:
    bool getConnected();
    bool getFinished();
    bool setConnected(bool b);
    bool setFinished(bool b);
	void readFromKeyboard(ConnectionHandler& connectionHandler);
	void communicateWithSocket(ConnectionHandler& connectionHandler);
    ConnectionHandler* getConnectionHandler();
	virtual ~client();
	client(ConnectionHandler* connectionHandler);
    void sendPacket(ServerPacket* packet);
    void printPacket(ServerPacket* packet);
    string popString(vector<char>& bytes);
	void printChar(char c);
    void sendVecViaCH(vector<char>& dataToSendVec);

private:

	ConnectionHandler* connectionHandler;
	bool connected=false;
	bool finished=false;
    char* vecToArr (vector<char>& v);
    vector<char>& arrToVec (char* c);


};

class readFromKeyboardTask {
private:
    client client_;
//    boost::mutex * _mutex;
public:
    readFromKeyboardTask(client client);
    void run();
    void operator()();
	void printChar(char c);

};


class communicateWithServerTask {
private:
    client client_;
//    boost::mutex * _mutex;
public:
	communicateWithServerTask(client client);
    void run();
    string popString(vector<char>& bytes);

    void printPacket(ServerPacket* packet) ;
	void printChar(char c);
    void operator()();
};

#endif /* BOOST_ECHO_CLIENT_SRC_CLIENT_H_ */
