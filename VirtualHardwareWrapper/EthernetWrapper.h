// EthernetWrapper.h

#pragma once

//using namespace System;

class EthernetWrapperPrivate;

class __declspec(dllexport) EthernetWrapper
{
private: EthernetWrapperPrivate* _private;

public: EthernetWrapper();

public: ~EthernetWrapper();

public: const char* clientErrorMessage(int socketNumber);

public: const char* clientSocketErrorCode(int socketNumber);

public: long clientErrorCode(int socketNumber);

public: int clientConnect(const char *hostname, unsigned int port, int *socketNumber);

public: int clientAvailable(int socketNumber);

public: unsigned char clientConnected(int socketNumber);

public: int clientPeek(int socketNumber);

public: void clientFlush(int socketNumber);

public: void clientStop(int socketNumber);

public: unsigned char clientStatus(int socketNumber);

public: void clientClose(int socketNumber);

public: unsigned int clientWrite(int socketNumber, const unsigned char *buf, unsigned int size);

public: int clientRead(int socketNumber, unsigned char *buf, unsigned int bytes);

public: const char* clientRemoteIpAddress(int socketNumber);

public: unsigned int clientRemotePort(int socketNumber);

//---------------------------------------------

public: const char* serverErrorMessage(int socketNumber);

public: const char* serverSocketErrorCode(int socketNumber);

public: long serverErrorCode(int socketNumber);

public: int serverBegin(const char *ipAddress, unsigned int port, int *socketNumber);

public: int serverAccept(int socketNumber);

// ---------------------------------------------

public: int udpBegin(unsigned int port, int *socketNumber);

public: int udpBeginPacket(int socketNumber, const char *hostname, unsigned int port);

public: int udpEndPacket(int socketNumber);

public: int udpParsePacket(int socketNumber, unsigned int *remoteIpAddress, unsigned int *remotePort);

public: unsigned int udpWrite(int socketNumber, const unsigned char *buf, unsigned int size);

public: int udpRead(int socketNumber, unsigned char *buf, unsigned int bytes);

public: int udpPeek(int socketNumber);

public: void udpClose(int socketNumber);
		
};
