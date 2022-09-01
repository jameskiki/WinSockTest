#include "TCP.h"

TCP::TCP(PCSTR ip, int port)
{
	WSADATA wsa;
	printf("Initialising Winsock...\n");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)	// init version 2.2
	{
		printf("Failed. Error Code : %d\n", WSAGetLastError());
	}
	printf("Initialised.\n");

	printf("creating Socket...\n");

	if ((own_sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)	// IPv4 Socket using TCP
	{
		printf("Could not create socket : %d\n", WSAGetLastError());
	}
	else
	{
		printf("Socket created.\n");
	}

	int alivetgl = 1;
	setsockopt(own_sock, SOL_SOCKET, SO_KEEPALIVE, (char*)&alivetgl, sizeof(alivetgl));

	memset(&si_me, 0, sizeof(si_me));
	if (!inet_pton(AF_INET, ip, &si_me.sin_addr.s_addr))
	{
		printf("failed to set server IP.\n");
		//return;
	}
	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(port);
}

TCP::~TCP()
{
}

bool TCP::ConnectToTarget(PCSTR targ_ip, int targ_port)
{
	memset(&si_target, 0, sizeof(si_target));
	if (!inet_pton(AF_INET, targ_ip, &si_target.sin_addr.s_addr))
	{
		printf("failed to set server IP.\n");
		//return;
	}
	si_target.sin_family = AF_INET;
	si_target.sin_port = htons(targ_port);

	char buff[255];
	auto ip_is = inet_ntop(AF_INET, &si_target.sin_addr.S_un.S_addr, buff, 255);
	printf("Set peer info: ip %s, port %i\n", ip_is, targ_port);

	//Connect to remote server

	printf("attempting to connect to peer...\n");
	if (connect(own_sock, (struct sockaddr*)&si_target, sizeof(si_target)) == SOCKET_ERROR)
	{
		printf("Could not connect to server : %d\n", WSAGetLastError());
		return false;
	}
	else
	{
		puts("Connected\n");
		return true;
	}
}

bool TCP::StartListening()
{
	char buff[255];
	auto ip_is = inet_ntop(AF_INET, &si_me.sin_addr.S_un.S_addr, buff, 255);
	printf("Binding to: ip %s, port %i\n", ip_is, ntohs(si_me.sin_port));
	//Bind
	if (bind(own_sock, (struct sockaddr*)&si_me, sizeof(si_me)) == SOCKET_ERROR)
	{
		auto error = WSAGetLastError();
		switch (error)
		{
		case WSAEAFNOSUPPORT:
			printf("Address family not supported by protocol family\n");
			break;
		default:
			break;
		}
		printf("bind failed with error code : %d", error);
		return false;
	}

	puts("Bind done");

	//Listen for incoming connections
	if (listen(own_sock, 3) == SOCKET_ERROR)
	{
		printf("Listen failed with error code : %d", WSAGetLastError());
		return false;
	}

	int c = sizeof(struct sockaddr_in);
	client_sock = accept(own_sock, (struct sockaddr*)&si_target, &c);
	if (client_sock == INVALID_SOCKET)
	{
		printf("accept failed with error code : %d\n", WSAGetLastError());
		return false;
	}
	printf("Connection accepted, Client scoket is: %i\n", client_sock);
	return true;
}

void TCP::DisconnectFromTarget()
{
	if(client_sock != INVALID_SOCKET)
		closesocket(client_sock);
	
	if (own_sock != INVALID_SOCKET)
		closesocket(own_sock);
}


bool TCP::SendChars(SOCKET sock, const char* buf, int len)
{
	const int maxMsgLen = 1023;

	if (sock == INVALID_SOCKET)
	{
		printf("Sending socket is invalid...\n");
		return false;
	}

	int n_loop = len / maxMsgLen;

	int i;
	for(i = 0; i <= n_loop; i++)
	{
		int it_len = ((len > maxMsgLen) ? maxMsgLen : len);

		char* nextStartPtr = (char*) &buf[i * maxMsgLen];

		if (send(sock, nextStartPtr, it_len, 0) == SOCKET_ERROR)
		{
			auto error = WSAGetLastError();
			switch (error)
			{
			case WSAENOTSOCK:
				printf("send failed with, not a socket\n");
				break;
			case WSAENOTCONN:
				printf("recv failed, Socket is not connected\n");
				break;
			default:
				printf("send failed with error code : %d\n", error);
				break;
			}
			return false;
		}
		len -= maxMsgLen;
	}
	return true;
}

bool TCP::SendString(SOCKET sock, std::string msg)
{
	return SendChars(sock, msg.c_str(), msg.length());
}

char* TCP::ReceiveFromSocketString(SOCKET sock, int * toatl_len)
{
	if (sock == INVALID_SOCKET)
	{
		printf("Receiving socket is invalid...\n");
		return nullptr;
	}

	// set receive timeout for socket
	DWORD timeout = 0.25 * 1000;
	auto retval = setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof timeout);

	char buf[BUFLEN];
	char* outbuf = nullptr;
	//clear the buffer by filling null, it might have previously received data
	memset(buf, '\0', BUFLEN);
	int recv_size = 0;
	int total_size = 0;
	//Receive a reply from the server
	int i = -1, timeoutcntr = 0;
	bool timeoutLimReached = false;
	do
	{
		if ((recv_size = recv(sock, buf, BUFLEN, 0)) == SOCKET_ERROR)
		{
			auto error = WSAGetLastError();
			switch (error)
			{
			case WSAETIMEDOUT:
				printf("recv failed, timed out\n");
				if (timeoutcntr > 1)
				{
					timeoutLimReached = true;
				}
				timeoutcntr++;
				recv_size = BUFLEN;
				break;
			case WSAENOTSOCK:
				printf("recv failed, not a socket\n");
				break;
			case WSAENOTCONN:
				printf("recv failed, Socket is not connected\n");
				recv_size = -1;
				break;
			default:
				printf("recv failed with error code : %d", WSAGetLastError());
				exit(EXIT_FAILURE);
				break;
			}
		}
		total_size += recv_size;

		// copy from small buffer to enlarged buffer
		char* newBuff = new char[total_size];
		// copy current outbuffer int new buffer
		std::memcpy(&newBuff[0], &outbuf[0], total_size - recv_size);
		// copy received buffer int new buffer
		std::memcpy(&newBuff[total_size - recv_size], &buf[0], recv_size);
		delete outbuf;
		outbuf = newBuff;

		i += 1;
	} while (!timeoutLimReached && recv_size == BUFLEN );

	*toatl_len = i * BUFLEN + recv_size;
	
	printf("Received %i bytes\n", *toatl_len);

/*	for (int i = 0; i < i * BUFLEN + recv_size - 1; i++)
	{
		printf("%c", outbuf[i]);
	}
	printf("\n");*/
	return outbuf;
}
