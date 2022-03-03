#pragma once


#ifndef RPI // on windows
#include <winsock2.h>
#include <Ws2tcpip.h>
#pragma comment(lib,"ws2_32.lib") //Winsock Library
#else	// runing on rpi

#endif

#include <iostream>

class SocketWrapper
{
public:
	SocketWrapper();
	~SocketWrapper();

	void TestServer();
	void TestClient();


};

// ERROR Codes https://docs.microsoft.com/en-us/windows/win32/winsock/windows-sockets-error-codes-2
// Guid for sockets https://www.binarytides.com/winsock-socket-programming-tutorial/
// cmd to check for open port "netstat -a -o | find "1111""