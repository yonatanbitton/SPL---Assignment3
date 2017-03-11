#ifndef CONNECTION_HANDLER__
#define CONNECTION_HANDLER__
                                           
#include <string>
#include <iostream>
#include <boost/asio.hpp>
#include "../include/ClientProtocol.h"
#include "../include/EncoderDecoder.h"
using namespace std;

using boost::asio::ip::tcp;

class ClientProtocol;

class ConnectionHandler {
private:
	const std::string host_;
	const short port_;
	boost::asio::io_service io_service_;   // Provides core I/O functionality
	tcp::socket socket_;
    ClientProtocol* protocol;
    EncoderDecoder* encdec;
    bool connected;
    void printChar(const char c) const;
    void printBytes(const char bytes[], int size) const;
public:
    ConnectionHandler(string host, short port);
    virtual ~ConnectionHandler();
    ClientProtocol* getProtocol();
    EncoderDecoder* getEncdec();
    void manualInitializing();
    bool connect();
    void setConnectionState(bool b);
    bool getConnectionState();


        // Read a fixed number of bytes from the server - blocking.
    // Returns false in case the connection is closed before bytesToRead bytes can be read.
    bool getBytes(char bytes[], unsigned int bytesToRead);
 
	// Send a fixed number of bytes from the client - blocking.
    // Returns false in case the connection is closed before all the data is sent.
    bool sendBytes(const char bytes[], int bytesToWrite);
	
    // Read an ascii line from the server
    // Returns false in case connection closed before a newline can be read.
    bool getLine(std::string& line);
	
	// Send an ascii line from the server
    // Returns false in case connection closed before all the data is sent.
    bool sendLine(std::string& line);
 
    // Get Ascii data from the server until the delimiter character
    // Returns false in case connection closed before null can be read.
    bool getFrameAscii(std::string& frame, char delimiter);
 
    // Send a message to the remote host.
    // Returns false in case connection is closed before all the data is sent.
    bool sendFrameAscii(const std::string& frame, char delimiter);
	
    // Close down the connection properly.
    void close();

    void printIntention(const char bytes[], int num);

    }; //class ConnectionHandler
 
#endif