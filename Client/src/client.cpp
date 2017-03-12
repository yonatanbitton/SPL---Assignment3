
#include "../include/client.h"
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
        vector <char> dataVecToSend;
        char nextChar;
        char dataCharArr[1];
        int index=0;
		while (connectionHandler->getConnectionState()==true){
			if (connectionHandler->getBytes(dataCharArr, 1)) {
                nextChar = dataCharArr[0]; //getBytes gets a char*, but actually returns char

                ServerPacket *receivedPacket = encdec->decodeNextByte((char) nextChar,connectionHandler); // Returns null if not finished
                if (receivedPacket != nullptr) {
                    index=0;
                    protocol_ptr->process(receivedPacket, connectionHandler);
                }
                index++;
            }
            else
            {
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
    }



	char* client::vecToArr(vector<char>& v){
		char* buf = &v[0];
	    return buf;
	}

    ConnectionHandler* client::getConnectionHandler(){
        return connectionHandler;
    }

client::~client() {
//    delete(connectionHandler);
}

string client::popString(vector<char>& bytes) {
    std::string s(bytes.begin(), bytes.end());
    return s;

}


string communicateWithServerTask::popString(vector<char>& bytes) {
    //notice that we explicitly requesting that the string will be decoded from UTF-8
    //this is not actually required as it is the default encoding in java.

    std::string s(bytes.begin(), bytes.end());
    return s;

}


//
int main (int argc, char *argv[]) {
    cout << "CHIN" << endl;
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

