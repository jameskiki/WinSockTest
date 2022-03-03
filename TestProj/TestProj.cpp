// TestProj.cpp : This file contains the 'main' function. Program execution begins and ends there.
//


#include <stdio.h>
#include <iostream>
#include <string>
#include <chrono>
#include <thread>

#include <winsock2.h>
#include <Ws2tcpip.h>
#pragma comment(lib,"ws2_32.lib") //Winsock Library

void client_task_TCP();
void server_task_TCP();

void client_task_UDP();
void server_task_UDP();

using namespace  std::chrono_literals;

#define CLIENT_IP "127.0.0.1"
#define SERVER_IP  "160.85.93.203"
#define BUFLEN 512	//Max length of buffer
#define PORT 3000	//The port on which to listen for incoming data


bool stop = false;

int main()
{/*
	TestFnc();

	return 0;*/

	printf("\nMAIN sarting server thread...\n");
	std::thread server(server_task_UDP);
	std::this_thread::sleep_for(2s);
	printf("\nMAIN sarting client thread...\n");
	std::thread client(client_task_UDP);

	//server.join();
	client.join();
	printf("MAIN terminating execution...\n");
}



//*******************
//		UDP
//*******************

void client_task_UDP() {
	printf("starting Client UDP\n");
	struct sockaddr_in si_other;
	int s, slen = sizeof(si_other);
	char buf[BUFLEN];
	char message[BUFLEN];
	WSADATA wsa;

	//Initialise winsock
	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	printf("Initialised.\n");

	//create socket
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == SOCKET_ERROR)
	{
		printf("socket() failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}

	sockaddr_in my_socket_addr;
	memset(&my_socket_addr, 0, sizeof(my_socket_addr));
	my_socket_addr.sin_family = AF_INET;
	my_socket_addr.sin_port = htons(PORT);
	inet_pton(AF_INET, "160.85.110.60", &my_socket_addr.sin_addr.S_un.S_addr);

	if (bind(s, (struct sockaddr*)&my_socket_addr, sizeof(my_socket_addr)) == SOCKET_ERROR)
	{
		printf("CLIENT Bind failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}

	//setup address structure for server
	memset((char*)&si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(PORT);
	if (!inet_pton(AF_INET, SERVER_IP, &si_other.sin_addr.S_un.S_addr)) {
		printf("CLIENT could not set IP");
	}


	// inform server that client is alive
	//send the message
	std::string msg = "hello pat";
	if (sendto(s, msg.c_str(), msg.length(), 0, (struct sockaddr*)&si_other, slen) == SOCKET_ERROR)
	{
		printf("sendto() failed with error code : %d", WSAGetLastError());
	}


	//start communication
	while (1)
	{
		//printf("Enter message : ");
		//std::cin >> message;

		////send the message
		//if (sendto(s, message, strlen(message), 0, (struct sockaddr*)&si_other, slen) == SOCKET_ERROR)
		//{
		//	printf("sendto() failed with error code : %d", WSAGetLastError());
		//}

		//receive a reply and print it
		//clear the buffer by filling null, it might have previously received data
		memset(buf, '\0', BUFLEN);
		//try to receive some data, this is a blocking call
		if (recvfrom(s, buf, BUFLEN, 0, (struct sockaddr*)&si_other, &slen) == SOCKET_ERROR)
		{
			printf("recvfrom() failed with error code : %d", WSAGetLastError());
		}

		puts(buf);
	}

	closesocket(s);
	WSACleanup();
}

void server_task_UDP() {
	printf("starting Server UDP\n");

	SOCKET s;
	struct sockaddr_in server, si_other;
	int slen, recv_len;
	char buf[BUFLEN];
	WSADATA wsa;

	slen = sizeof(si_other);

	//Initialise winsock
	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	printf("Initialised.\n");

	//Create a socket
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d", WSAGetLastError());
	}
	printf("Socket created.\n");

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(PORT);

	//Bind
	if (bind(s, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
	{
		printf("Bind failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	puts("Bind done");

	//keep listening for data
	while (1)
	{
		printf("Waiting for data...");
		fflush(stdout);

		//clear the buffer by filling null, it might have previously received data
		memset(buf, '\0', BUFLEN);

		//try to receive some data, this is a blocking call
		if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr*)&si_other, &slen)) == SOCKET_ERROR)
		{
			printf("recvfrom() failed with error code : %d", WSAGetLastError());
			exit(EXIT_FAILURE);
		}

		//print details of the client/peer and the data received
		char addr[25];
		inet_ntop(AF_INET, &si_other.sin_addr, addr, 25);
		printf("Received packet from %s:%d\n",addr, ntohs(si_other.sin_port));
		printf("Data: %s\n", buf);

		//now reply the client with the same data
		if (sendto(s, buf, recv_len, 0, (struct sockaddr*)&si_other, slen) == SOCKET_ERROR)
		{
			printf("sendto() failed with error code : %d", WSAGetLastError());
			exit(EXIT_FAILURE);
		}
	}

	closesocket(s);
	WSACleanup();
}




//*******************
//		TCP	
//*******************

void client_task_TCP() {
	WSADATA wsa;
	printf("starting Client TCP\n");
	printf("CLIENT Initialising Winsock...\n");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)	// init version 2.2
	{
		printf("CLIENT Failed. Error Code : %d\n", WSAGetLastError());
	}

	printf("CLIENT Initialised.\n");

	printf("CLIENT creating Socket...\n");
	SOCKET s;

	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)	// IPv4 Socket using TCP
	{
		printf("CLIENT Could not create socket : %d\n", WSAGetLastError());
	}
	else
	{
		printf("CLIENT Socket created.\n");
	}

	// connect to server
	struct sockaddr_in server;
	memset(&server, 0, sizeof(server));
	if (!inet_pton(AF_INET, (PCSTR)SERVER_IP, &server.sin_addr.s_addr))
	{
		printf("CLIENT failed to set server IP.\n");
		//return;
	}
	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);

	printf("CLIENT setup as: ip %u \t port %i\n", server.sin_addr.s_addr, PORT);

	//Connect to remote server

	printf("CLIENT attempting to connect to server...\n");
	if (connect(s, (struct sockaddr*)&server, sizeof(server)) < 0)
	{
		printf("CLIENT Could not connect to server : %d\n", WSAGetLastError());
	}
	else
	{
		puts("CLIENT Connected\n");
	}

	char* message, server_reply[2000];
	int recv_size;
	//Receive a reply from the server
	if ((recv_size = recv(s, server_reply, 2000, 0)) == SOCKET_ERROR)
	{
		puts("CLIENT recv failed");
		return;
	}

	puts("CLIENT Reply received\n");

	//Add a NULL terminating character to make it a proper string before printing
	server_reply[recv_size] = '\0';
	puts(server_reply);

	closesocket(s);
	WSACleanup();

	return;
}

void server_task_TCP() {
	WSADATA wsa;
	SOCKET s, client_socket;
	struct sockaddr_in client;
	int c;
	std::string message;

	printf("starting Server TCP\n");
	printf("SERVER Initialising Winsock...\n");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("SERVER Failed. Error Code : %d\n", WSAGetLastError());
		return;
	}

	printf("SERVER Initialised.\n");

	//Create a socket
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		printf("SERVER Could not create socket : %d", WSAGetLastError());
	}

	printf("SERVER Socket created.\n");

	//Prepare the sockaddr_in structure
	struct sockaddr_in server;
	memset(&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	if (!inet_pton(AF_INET, (PCSTR)SERVER_IP, &server.sin_addr.s_addr))
	{
		printf("CLIENT failed to set server IP.\n");
		//return;
	}
	server.sin_port = htons(PORT);

	printf("SERVER setup as: ip %u \t port %i\n", server.sin_addr.s_addr, PORT);

	//Bind
	if (bind(s, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
	{
		printf("SERVER Bind failed with error code : %d", WSAGetLastError());
	}

	puts("SERVER Bind done");

	//Listen to incoming connections
	listen(s, 3);

	//Accept and incoming connection
	puts("SERVER Waiting for incoming connections...");

	c = sizeof(struct sockaddr_in);
	client_socket = accept(s, (struct sockaddr*)&client, &c);
	if (client_socket == INVALID_SOCKET)
	{
		printf("SERVER accept failed with error code : %d\n", WSAGetLastError());
	}

	puts("SERVER Connection accepted\n");

	message = "Msg from Server to Client:  Hello Client , I have received your connection. But I have to go now, bye\n";
	send(client_socket, message.c_str(), strlen(message.c_str()), 0);

	closesocket(s);
	WSACleanup();

	return;
}






// Old stuff ##################################################

void memcpy_test() {
    float var = 2.5f;
    float var2 = 0.0f;
    uint8_t buf[4];

    memcpy(buf, &var, 4);
    memcpy(&var2, buf, 4);
}