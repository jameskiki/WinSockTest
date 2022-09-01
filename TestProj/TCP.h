#pragma once

#include <string>

#include <winsock2.h>
#include <Ws2tcpip.h>
#pragma comment(lib,"ws2_32.lib") //Winsock Library

#define BUFLEN 1023	//Max length of buffer

class TCP
{
public:
	TCP(PCSTR ip, int port);
	~TCP();
	bool ConnectToTarget(PCSTR targ_ip, int targ_port);
	bool StartListening();
	void DisconnectFromTarget();
	bool SendChars(SOCKET sock, const char* buf, int len);
	bool SendString(SOCKET sock, std::string msg);
	char* ReceiveFromSocketString(SOCKET socket, int* toatl_len);

	SOCKET own_sock = INVALID_SOCKET, client_sock = INVALID_SOCKET;

private:
	struct sockaddr_in si_me, si_target;
};

