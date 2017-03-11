cmake_minimum_required(VERSION 3.6)
project(Client)

set(CMAKE_CXX_STANDARD 11)
find_package (Boost 1.58.0 COMPONENTS system filesystem thread REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
include_directories(Client/include/)


set(SOURCE_FILES
        include/client.h
        include/ClientProtocol.h
        include/ConnectionHandler.h
        include/EncoderDecoder.h
        include/ServerPacket.h
        src/client.cpp
        src/ClientProtocol.cpp
        src/ConnectionHandler.cpp
        src/EncoderDecoder.cpp
        src/ServerPacket.cpp)

add_executable(Client ${SOURCE_FILES})
TARGET_LINK_LIBRARIES( Client ${Boost_LIBRARIES} )

