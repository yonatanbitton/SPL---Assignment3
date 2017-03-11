# define some Makefile variables for the compiler and compiler flags
# to use Makefile variables later in the Makefile: $()
CC = g++
CFLAGS  = -g -Wall -Weffc++ -std=c++11
LFLAGS  = -L/usr/lib -lboost_system -lboost_locale -lpthread -lboost_thread

# All Targets
all: TFTPclient

# Tool invocations
TFTPclient: bin/ServerPacket.o bin/ConnectionHandler.o bin/EncoderDecoder.o bin/ClientProtocol.o bin/TFTPclient.o
	@echo 'Building target: TFTPclient'
	@echo 'Invoking: C++ Linker'
	$(CC) -o bin/TFTPclient bin/ServerPacket.o bin/ConnectionHandler.o bin/EncoderDecoder.o bin/ClientProtocol.o bin/TFTPclient.o $(LFLAGS)
	@echo 'Finished building target: TFTPclient'
	@echo ' '

bin/ServerPacket.o: src/ServerPacket.cpp
	$(CC) $(CFLAGS) -c -Iinclude -o bin/ServerPacket.o src/ServerPacket.cpp

bin/ConnectionHandler.o: src/ConnectionHandler.cpp
	$(CC) $(CFLAGS) -c -Iinclude -o bin/ConnectionHandler.o src/ConnectionHandler.cpp

bin/EncoderDecoder.o: src/EncoderDecoder.cpp
	$(CC) $(CFLAGS) -c -Iinclude -o bin/EncoderDecoder.o src/EncoderDecoder.cpp

bin/ClientProtocol.o: src/ClientProtocol.cpp
	$(CC) $(CFLAGS) -c -Iinclude -o bin/ClientProtocol.o src/ClientProtocol.cpp

bin/TFTPclient.o: src/client.cpp
	$(CC) $(CFLAGS) -c -Iinclude -o bin/TFTPclient.o src/client.cpp

#Clean the build directory
clean:
	rm -f bin/*
