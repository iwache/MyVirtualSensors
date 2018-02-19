// EthernetWrapper.cpp

//#using "..\VirtualHardwareNet\bin\Debug\VirtualHardwareNet.dll"
//#using "VirtualHardwareNet.dll"

#include <msclr\auto_gcroot.h>
#include "EthernetWrapper.h"

using namespace System::Runtime::InteropServices; // Marshal
using namespace VirtualHardwareNet;


class EthernetWrapperPrivate
{
	public: msclr::auto_gcroot<EthernetNet^> ethernet;
};

EthernetWrapper::EthernetWrapper()
{
	_private = new EthernetWrapperPrivate();
	_private->ethernet = gcnew EthernetNet();
}

EthernetWrapper::~EthernetWrapper()
{
	delete _private;
}

const char* EthernetWrapper::clientErrorMessage(int socketNumber)
{
	System::String^ errorMessage = _private->ethernet->clientErrorMessage(socketNumber);
	return (const char*)Marshal::StringToHGlobalAnsi(errorMessage).ToPointer();
}

const char* EthernetWrapper::clientSocketErrorCode(int socketNumber)
{
	System::String^ socketErrorCode = _private->ethernet->clientSocketErrorCode(socketNumber);
	return (const char*)Marshal::StringToHGlobalAnsi(socketErrorCode).ToPointer();
}

long EthernetWrapper::clientErrorCode(int socketNumber)
{
	return _private->ethernet->clientErrorCode(socketNumber);
}

int EthernetWrapper::clientConnect(const char *hostname, unsigned int port, int *socketNumber)
{
	int sock;
	int result = _private->ethernet->clientConnect(gcnew System::String(hostname), port, sock);
	*socketNumber = sock;
	return result;
}

int EthernetWrapper::clientAvailable(int socketNumber)
{
	return _private->ethernet->clientAvailable(socketNumber);
}

unsigned char EthernetWrapper::clientConnected(int socketNumber)
{
	return _private->ethernet->clientConnected(socketNumber);
}

int EthernetWrapper::clientPeek(int socketNumber)
{
	return _private->ethernet->clientPeek(socketNumber);
}

void EthernetWrapper::clientFlush(int socketNumber)
{
	_private->ethernet->clientFlush(socketNumber);
}

void EthernetWrapper::clientStop(int socketNumber)
{
	_private->ethernet->clientStop(socketNumber);
}

unsigned char EthernetWrapper::clientStatus(int socketNumber)
{
	return _private->ethernet->clientStatus(socketNumber);
}

void EthernetWrapper::clientClose(int socketNumber)
{
	_private->ethernet->clientClose(socketNumber);
}

unsigned int EthernetWrapper::clientWrite(int socketNumber, const unsigned char *buf, unsigned int size)
{
	array<unsigned char>^ data = gcnew array<unsigned char>(size);
	Marshal::Copy(System::IntPtr((void *)buf), data, 0, size);
	return _private->ethernet->clientWrite(socketNumber, data, size);
}

int EthernetWrapper::clientRead(int socketNumber, unsigned char *buf, unsigned int bytes)
{
	array<unsigned char>^ data = gcnew array<unsigned char>(bytes);
	int result = _private->ethernet->clientRead(socketNumber, data, bytes);
	Marshal::Copy(data, 0, System::IntPtr((void *)buf), result);
	return result;
}

const char* EthernetWrapper::clientRemoteIpAddress(int socketNumber)
{
	System::String^ remoteIpAddress = _private->ethernet->clientRemoteIpAddress(socketNumber);
	return (const char*)Marshal::StringToHGlobalAnsi(remoteIpAddress).ToPointer();
}

unsigned int EthernetWrapper::clientRemotePort(int socketNumber)
{
	return _private->ethernet->clientRemotePort(socketNumber);
}

// ---------------------------------------------------------

const char* EthernetWrapper::serverErrorMessage(int socketNumber)
{
	System::String^ errorMessage = _private->ethernet->serverErrorMessage(socketNumber);
	return (const char*)Marshal::StringToHGlobalAnsi(errorMessage).ToPointer();
}

const char* EthernetWrapper::serverSocketErrorCode(int socketNumber)
{
	System::String^ socketErrorCode = _private->ethernet->serverSocketErrorCode(socketNumber);
	return (const char*)Marshal::StringToHGlobalAnsi(socketErrorCode).ToPointer();
}

long EthernetWrapper::serverErrorCode(int socketNumber)
{
	return _private->ethernet->serverErrorCode(socketNumber);
}

int EthernetWrapper::serverBegin(const char *ipAddress, unsigned int port, int *socketNumber)
{
	int sock;
	int result = _private->ethernet->serverBegin(gcnew System::String(ipAddress), port, sock);
	*socketNumber = sock;
	return result;
}

int EthernetWrapper::serverAccept(int socketNumber)
{
	return _private->ethernet->serverAccept(socketNumber);
}

// ---------------------------------------------------------

int EthernetWrapper::udpBegin(unsigned int port, int *socketNumber)
{
	int sock;
	int result = _private->ethernet->udpBegin(port, sock);
	*socketNumber = sock;
	return result;
}

int EthernetWrapper::udpBeginPacket(int socketNumber, const char *hostname, unsigned int port)
{
	int result = _private->ethernet->udpBeginPacket(socketNumber, gcnew System::String(hostname), port);
	return result;
}

int EthernetWrapper::udpEndPacket(int socketNumber)
{
	return _private->ethernet->udpEndPacket(socketNumber);
}

int EthernetWrapper::udpParsePacket(int socketNumber, unsigned int *remoteIpAddress, unsigned int *remotePort)
{
	unsigned short port;
	unsigned int ipAddress; 
	int result = _private->ethernet->udpParsePacket(socketNumber, ipAddress, port);
	*remoteIpAddress = ipAddress;
	*remotePort = port;
	return result;
}

unsigned int EthernetWrapper::udpWrite(int socketNumber, const unsigned char *buf, unsigned int size)
{
	array<unsigned char>^ data = gcnew array<unsigned char>(size);
	Marshal::Copy(System::IntPtr((void *)buf), data, 0, size);
	return _private->ethernet->udpWrite(socketNumber, data, size);
}

int EthernetWrapper::udpRead(int socketNumber, unsigned char *buf, unsigned int bytes)
{
	array<unsigned char>^ data = gcnew array<unsigned char>(bytes);
	int result = _private->ethernet->udpRead(socketNumber, data, bytes);
	Marshal::Copy(data, 0, System::IntPtr((void *)buf), result);
	return result;
}

int EthernetWrapper::udpPeek(int socketNumber)
{
	return _private->ethernet->udpPeek(socketNumber);
}

void EthernetWrapper::udpClose(int socketNumber)
{
	_private->ethernet->udpClose(socketNumber);
}

