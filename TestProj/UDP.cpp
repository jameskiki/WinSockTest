#include "UDP.h"

UDP::UDP(PCSTR ip, int port)
{
	WSADATA wsa;
	//Initialise winsock
	printf("Initialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	printf("Initialised.\n");

	//create socket
	if ((own_sock = socket(AF_INET, SOCK_DGRAM, 0)) == SOCKET_ERROR)
	{
		printf("socket() failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}

	memset(&si_me, 0, sizeof(si_me));
	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(port);
	inet_pton(AF_INET, ip, &si_me.sin_addr.S_un.S_addr);

	auto retval = bind(own_sock, (struct sockaddr*)&si_me, sizeof(si_me));
	if (retval == SOCKET_ERROR)
	{
		auto error = WSAGetLastError();
		switch (error)
		{
		case WSAEADDRNOTAVAIL:
			printf("bind failed, Adress not available\n");
			exit(error);
			break;
		case WSAEADDRINUSE:
			printf("bind failed, Adress in use\n");
			exit(error);
			break;
		default:
			printf("bind failed with error code : %d", WSAGetLastError());
			exit(EXIT_FAILURE);
			break;
		}
	}
	{
		char buff[255];
		auto ip_is = inet_ntop(AF_INET, &si_me.sin_addr.S_un.S_addr, buff, 255);
		printf("Set own info: ip %s, port %i\n", ip_is, port);
	}
	printf("socket bound...\n");
}

UDP::~UDP()
{
	WSACleanup();
}

bool UDP::ConnectToTarget(PCSTR targ_ip, int targ_port)
{
	//setup address structure for server
	memset((char*)&si_target, 0, sizeof(si_target));
	si_target.sin_family = AF_INET;
	si_target.sin_port = htons(targ_port);
	if (!inet_pton(AF_INET, targ_ip, &si_target.sin_addr.S_un.S_addr)) {
		printf("could not set IP of server\n");
		return false;
	}
	else
	{
		char buff[255];
		auto ip = inet_ntop(AF_INET, &si_target.sin_addr.S_un.S_addr, buff, 255);
		printf("Set target info for server: ip %s, port %i\n", ip, ntohs(si_target.sin_port));
		return true;
	}
}

int UDP::DisconnectFromTarget()
{
	if (closesocket(own_sock) == SOCKET_ERROR)
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool UDP::SendString(std::string msg)
{
	auto retval = sendto(own_sock, msg.c_str(), msg.length(), 0, (struct sockaddr*)&si_target, sizeof(si_target));
	if (retval == SOCKET_ERROR)
	{
		auto error = WSAGetLastError();
		switch (error)
		{
		case WSAEADDRNOTAVAIL:
			printf("sendto failed, Adress not available\n");
			exit(error);
			break;
		case WSAENETUNREACH:
			printf("sendto failed, Network not reachable\n");
			exit(error);
			break;
		default:
			printf("sendto failed with error code : %d", WSAGetLastError());
			exit(EXIT_FAILURE);
			break;
		}
		return false;
	}
	else
	{
		return true;
	}
}

std::string UDP::ReceiveString()
{
	char buf[BUFLEN];
	//clear the buffer by filling null, it might have previously received data
	memset(buf, '\0', BUFLEN);
	//try to receive some data, this is a blocking call
	int slen = sizeof(si_target);
	if (recvfrom(own_sock, buf, BUFLEN, 0, (struct sockaddr*)&si_target, &slen) == SOCKET_ERROR)
	{
		printf("recvfrom() failed with error code : %d", WSAGetLastError());
		return "";
	}
	else
	{
		char addr[25];
		inet_ntop(AF_INET, &si_target.sin_addr, addr, 25);
		printf("Received packet from %s:%d\n", addr, ntohs(si_target.sin_port));
		return buf;
	}
}
