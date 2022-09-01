// TestProj.cpp : This file contains the 'main' function. Program execution begins and ends there.
//


#include <stdio.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <thread>
#include <vector>


void sender_task();
void receiver_task();

void client_task_TCP();
void server_task_TCP();

void client_task_UDP();
void server_task_UDP();

using namespace  std::chrono_literals;

#define CLIENT_IP "127.0.0.1"
#define CLIENT_PORT 3000
#define SERVER_IP  "127.0.0.1"
#define SERVER_PORT 4000


#include "UDP.h"
#include "TCP.h"


//**********************
//		   temp
//**********************
void create_example_file() {
	std::ofstream file("SRC/file.ot");
	for (int i = 0; i < 1000; i++)
	{
		const char endl = '\n';
		char rep = 48 + i%10;
		file.write(&rep, 1);
		file.write(&endl, 1);
	}
	file.close();
}


bool stop = false;

int main()
{
	//create_example_file();
	printf("\n\n\n\MAIN sarting server thread...\n");
	std::thread task1(sender_task);
	//std::thread task1(server_task_UDP);
	//std::thread task1(server_task_TCP);

	std::this_thread::sleep_for(2s);

	printf("\n\n\nMAIN sarting client thread...\n");
	std::thread task2(receiver_task);
	//std::thread task2(client_task_UDP);
	//std::thread task2(client_task_TCP);

	task1.join();
	task2.join();
	printf("MAIN terminating execution...\n");
}

//**************************************
//		File transfer example
//**************************************

/*
* RECEIVER starts tcp-listener and informs SENDER that it is ready for file-transfer via tcp
* SENDER connects to tcp-listner on RECEIVER and transfers file;
*/

void sender_task() 
{
	printf("### starting sender task...\n");

	UDP udp_sock = UDP(CLIENT_IP, CLIENT_PORT);
	udp_sock.ConnectToTarget(SERVER_IP, SERVER_PORT);

	printf("### SENDER Waiting for data...");
	fflush(stdout);

	std::string msg = udp_sock.ReceiveString();
	printf("### SENDER received Data: %s\n", msg.c_str());

	// read the file
	std::stringstream content;
	std::ifstream file("SRC/file.ot", std::ios::binary);
	//get length of file
	file.seekg(0, std::ifstream::end);
	size_t length = file.tellg();
	file.seekg(0, std::ifstream::beg);

	char* buffer;
	buffer = new char[length];
	file.read(buffer, length);
	file.close();
	
	// setup tcp send
	TCP tcp_sock = TCP(CLIENT_IP, CLIENT_PORT + 1);
	tcp_sock.ConnectToTarget(SERVER_IP, SERVER_PORT + 1);

	printf("### SENDER sending file content...\n");
	tcp_sock.SendChars(tcp_sock.own_sock, buffer, length);

}

void receiver_task()
{
	printf("### starting RECEIVER task...\n");

	// basic communication way
	UDP udp_sock = UDP(SERVER_IP, SERVER_PORT);
	udp_sock.ConnectToTarget(CLIENT_IP, CLIENT_PORT);

	// setup tcp receive
	TCP tcp_sock = TCP(SERVER_IP, SERVER_PORT + 1);

	printf("### RECEIVER sending request...\n");
	udp_sock.SendString("pls send file.ot");

	// wait for response, hopfully containing file
	printf("### RECEIVER listening for connection request...\n");
	tcp_sock.StartListening();
	int total_length = -1;
	char* msg = tcp_sock.ReceiveFromSocketString(tcp_sock.client_sock, &total_length);

	std::stringstream ss(msg);
	std::ofstream file("DST/file.ot", std::ios::binary);
	file.write(msg, total_length);
}

//*******************
//		UDP
//*******************

void client_task_UDP() {
	printf("starting Client UDP\n");

	UDP udp_sock = UDP(CLIENT_IP, CLIENT_PORT);
	udp_sock.ConnectToTarget(SERVER_IP, SERVER_PORT);
	//start communication
	while (1)
	{
		// inform server that client is alive
		std::string msg = "hello server";
		//send the message
		udp_sock.SendString(msg);
		//receive a reply and print it
		auto resp = udp_sock.ReceiveString();
		if (resp != "")
		{
			break;
		}
		printf("CLIENT Retry...\n");
	}
	udp_sock.DisconnectFromTarget();
}

void server_task_UDP() {
	printf("starting Server UDP\n");

	UDP udp_sock = UDP(SERVER_IP, SERVER_PORT);
	udp_sock.ConnectToTarget(CLIENT_IP, CLIENT_PORT);

	//keep listening for data
	while (1)
	{
		printf("SERVER Waiting for data...");
		fflush(stdout);

		std::string msg = udp_sock.ReceiveString();
		printf("SERVER Data: %s\n", msg.c_str());

		//now reply the client with the same data
		if (udp_sock.SendString(msg))
		{
			break;
		}
		printf("SERVER Retry...\n");
	}

	udp_sock.DisconnectFromTarget();
}

//*******************
//		TCP	
//*******************

void client_task_TCP() {
	printf("starting CLIENT TCP\n");
	TCP tcp_sock = TCP(CLIENT_IP, CLIENT_PORT);
	// connect to server
	tcp_sock.ConnectToTarget(SERVER_IP, SERVER_PORT);	
	int len = -1;
	auto msg = tcp_sock.ReceiveFromSocketString(tcp_sock.own_sock, &len);

	//printf("received: %s", msg.c_str());
	WSACleanup();

	return;
}

void server_task_TCP() {
	printf("starting SERVER TCP\n");
	TCP tcp_sock = TCP(SERVER_IP, SERVER_PORT);

	//Accept and incoming connection
	puts("SERVER Listening for incoming connections...");
	if (tcp_sock.StartListening())
	{
		puts("SERVER connected...");
		std::string message = "Msg from Server to Client:  Hello Client , I have received your connection. But I have to go now, bye\n";
		tcp_sock.SendString(tcp_sock.client_sock, message);
	}

	tcp_sock.DisconnectFromTarget();

	return;
}
