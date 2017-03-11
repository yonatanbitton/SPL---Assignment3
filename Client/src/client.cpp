
#include "../include/client.h"
//#include <stdlib.h>
//#include "../include/ConnectionHandler.h"
//#include <string>
//#include <vector>
//#include "../include/ClientProtocol.h"
#include <boost/locale.hpp>
#include <boost/thread.hpp>
#include <boost/asio/ip/tcp.hpp>

class tcp;

using namespace std;
using boost::asio::ip::tcp;

    void readFromKeyboardTask::operator()() {
        run();
    }

    readFromKeyboardTask::readFromKeyboardTask(client cl_par) : client_(cl_par){}

    void communicateWithServerTask::operator()() {
        run();
    }

    communicateWithServerTask::communicateWithServerTask(client cl_par) : client_(cl_par){}

	// This function reads cmd from the keyboard, converts it to packet, encode it into vector, changes it into char*, and sends it to the server.
	void readFromKeyboardTask::run(){
        ConnectionHandler* connectionHandler = client_.getConnectionHandler();
        ClientProtocol* protocol_ptr = connectionHandler->getProtocol();

        while (connectionHandler->getConnectionState()==true) {
		        const short bufsize = 100;
		        char buf[bufsize];
		        std::cin.getline(buf, bufsize); // Getting the input from the keyboard
				std::string line(buf); // Now it's in the line
                if (line.length()==0) continue;
				ServerPacket *packet = protocol_ptr->processFromKeyboard(line);
                if (packet==nullptr || packet->getOpcode()==0) {}
//                    cout << "Invalid input from the keyboard" << endl;
                else {
                    client_.sendPacket(packet);
                    if (packet->getOpcode()==10) break;
                }
        }
    }



        // This function gets input from the server, converts it into vector, decode it to a packet, decides which packet to send according to the protocol
	void communicateWithServerTask::run(){
        ConnectionHandler* connectionHandler = client_.getConnectionHandler();
        ClientProtocol* protocol_ptr = connectionHandler->getProtocol();
        EncoderDecoder* encdec = connectionHandler->getEncdec();
//        encdec->printSize();
        vector <char> dataVecToSend;
        char nextChar;
        char dataCharArr[1];
//        encdec->manualInitialization();
        int index=0;
		while (connectionHandler->getConnectionState()==true){
			if (connectionHandler->getBytes(dataCharArr, 1)) {
                nextChar = dataCharArr[0]; //getBytes gets a char*, but actually returns char

                ServerPacket *receivedPacket = encdec->decodeNextByte((char) nextChar,connectionHandler); // Returns null if not finished
                if (receivedPacket != nullptr) {
                    index=0;
                    protocol_ptr->process(receivedPacket, connectionHandler);
                    //delete (receivedPacket);
                }
                index++;
            }
            else
            {
//                cout << "Getting the data from the server was failed " << endl;
            };

        }
    }

    client::client(ConnectionHandler* connectionHander_par):connectionHandler(connectionHander_par){}


    void client::sendVecViaCH(vector<char>& dataToSendVec){
        int size = dataToSendVec.size();
        char* dataToSendChar = vecToArr(dataToSendVec);
        connectionHandler->sendBytes(dataToSendChar, size);
    }

    void client::sendPacket(ServerPacket* packet){ // CLEAN THE FIELDS !
        EncoderDecoder* encdec = connectionHandler->getEncdec();
        char* toSendChar;
        vector<char>* toSendVec = encdec->encode(packet); // encode returns a pointer. So I create a new pointer to point to it.
        int size = toSendVec->size();
        encdec->cleanFields(); // VERY IMPORTANT
         toSendChar = vecToArr(*toSendVec); // I transfer the pointer, and the data inside it is changed. I point to the new change.
        connectionHandler->sendBytes(toSendChar, size); // sendBytes needs to get actual char[]
        //delete (toSendChar); // delete te pointer I transfered it's material
        //delete (toSendVec); // I delete the pointer returning from the encode
    }



	char* client::vecToArr(vector<char>& v){
		// Get a char pointer to the data in the vector
		char* buf = &v[0];
	    return buf;
	}

    ConnectionHandler* client::getConnectionHandler(){
        return connectionHandler;
    }

//
//	vector<char>& client::arrToVec(char* c){
//		int size=sizeof(c);
//		vector<char> v (c, c+size);
//		return v;
//	}

client::~client() {
//    delete(connectionHandler);
}

string client::popString(vector<char>& bytes) {
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


string communicateWithServerTask::popString(vector<char>& bytes) {
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


//
int main (int argc, char *argv[]) {
    cout << "CHIN" << endl;
    // Initialize host and port from the args.
//    if (argc < 3) {
//        std::cerr << "Usage: " << argv[0] << " host port" << std::endl << std::endl;
//        return -1;
//    }
//
//    std::string host = argv[1];
//    short port = atoi(argv[2]);
    string host="127.0.0.1";
    short port = 6666;

    ConnectionHandler connectionHandler(host, port);
    if (!connectionHandler.connect()) {
        std::cerr << "Cannot connect to " << host << ":" << port << std::endl;
        return 1;
    }

    connectionHandler.manualInitializing();
    ConnectionHandler* connectionHandler_ptr = &connectionHandler;
    client Yosi(connectionHandler_ptr);
    //Yosi.setConnected(true);
    connectionHandler_ptr->setConnectionState(true);

    readFromKeyboardTask rfkt(Yosi);
    communicateWithServerTask cwst(Yosi);

    // Thread for converting keyboard commands to data to send to the server, and another one to the socket communication.
    boost::thread keyboardThread(boost::ref(rfkt));
    boost::thread socketThread(boost::ref(cwst));

    keyboardThread.join();
    socketThread.join();


    return 0;
}

 /* namespace client */
