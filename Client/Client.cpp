#include "stdafx.h"
#include <stdio.h>
#include <conio.h>
#include <ws2tcpip.h>
#include <winsock2.h>
#include <process.h>

#pragma comment(lib, "Ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#define DEFAULT_PORT 5500
#define DEFAULT_ADDR "127.0.0.1"
#define BUFF_SIZE 512
#define MAX_CLIENT 10


int main(int argc, char* argv[])
{
	SOCKET clientSocket;
	WSADATA wsaData;

	//Step 1: Inittiate WinSock
	if (WSAStartup(MAKEWORD(2, 2), &wsaData)) {
		printf("Version is not supported!");
		_getch();
		return 0;
	}

	printf("----------------------Clinet command USER PASS and LOUT follow by space and message----------------------\n");
	printf("-----------------------Default USERNAME is username default PASSWORD is password-----------------------\n");
	printf("------------------------------Enter BYE! for exit and terminate connection------------------------------\n");

	//Step 2: Create socket	
	clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (clientSocket < 0) {
		printf("Error! Cannot create socket.");
		_getch();
		return 0;
	}

	//Step 3: Specify server address
	int port;
	char ipAddr[16];
	port = DEFAULT_PORT;
	strcpy(ipAddr, DEFAULT_ADDR);
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-p") == 0)	port = atoi(argv[++i]);
		if (strcmp(argv[i], "-a") == 0)	strcpy(ipAddr, argv[++i]);
	}


	SOCKADDR_IN serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.s_addr = inet_addr(ipAddr);
	printf("Connecting to %s:%d\n", ipAddr, port);

	//Step 4: Request to connect server
	if (connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)))
		printf("Cannot connect to server.");

	//Step 5: Communicate with server
	char buffer[BUFF_SIZE];
	int rcvBytes;
	memset(buffer, '0', BUFF_SIZE);

	//receive message from server
	while (1) {


		//scan user input
		printf("send to server: ");
		fgets(buffer, BUFF_SIZE, stdin);
		buffer[strcspn(buffer, "\n")] = 0;

		//sent user input to server
		send(clientSocket, buffer, BUFF_SIZE, 0);

		//terminate if get BYE!
		if (strncmp(buffer, "BYE!", 4) == 0)
			break;

		//receive message from server
		if ((rcvBytes = recv(clientSocket, buffer, BUFF_SIZE, 0)) < 0)
		{
			printf("Server close connection terminal shutdown\n");
			break;
		}

		//print outloud server message except code number
		printf("%s\n", buffer + 3);
	}

	WSACleanup();
	_getch();
	return 0;
}