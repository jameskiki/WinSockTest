#pragma once

#include <string>

#include <winsock2.h>
#include <Ws2tcpip.h>
#pragma comment(lib,"ws2_32.lib") //Winsock Library

#define BUFLEN 512	//Max length of buffer

class UDP
{
public:
	UDP(PCSTR ip, int port);
	~UDP();
	bool ConnectToTarget(PCSTR targ_ip, int targ_port);
	int DisconnectFromTarget();
	bool SendString(std::string msg);
	std::string ReceiveString();

private:
	SOCKET own_sock = INVALID_SOCKET;
	struct sockaddr_in si_me, si_target;


};

