// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <WinSock2.h>
#include <conio.h>
#include "mylib.h"

#define DEFAULT_PORT 5500
#define DEFAULT_ADDR "127.0.0.1"
#define QUEUE_SIZE 10

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "ws2_32.lib")


int main(int argc, char** argv) {
	int server_port = DEFAULT_PORT;
	char* server_addr = DEFAULT_ADDR;
	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "-p") == 0)
			server_port = atoi(argv[++i]);
	}

	DWORD nEvents = 0;
	DWORD index;
	struct client_info_struct clients[WSA_MAXIMUM_WAIT_EVENTS];
	memset(clients, 0, sizeof(struct client_info_struct) * WSA_MAXIMUM_WAIT_EVENTS);
	WSAEVENT events[WSA_MAXIMUM_WAIT_EVENTS], newEvent;
	WSANETWORKEVENTS sockEvent;
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsaData))	printf("Version is not supported\n");

	//listen sock
	SOCKET acceptSock, listenSock;
	listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//Bind address to socket
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(server_port);
	serverAddr.sin_addr.s_addr = inet_addr(server_addr);

	if (bind(listenSock, (sockaddr *)&serverAddr, sizeof(serverAddr))) {
		printf("Error! cannot bind to the address.\n");
		_getch();
		return 0;
	}

	printf("Server bind socket to port successful\n");

	if (listen(listenSock, QUEUE_SIZE)) {
		printf("Cannot listen at at listen sock\n");
		_getch();
		return 1;
	}

	//new events
	newEvent = WSACreateEvent();
	events[nEvents] = newEvent;
	clients[nEvents].client_fd = listenSock;
	clients[nEvents].is_active = 1;
	nEvents++;
	printf("Created socket event successful\n");

	//Associate event FD_ACCEPT and FD_CLOSE with listening socket
	WSAEventSelect(listenSock, newEvent, FD_ACCEPT | FD_CLOSE);
		while(1) {
		index = WSAWaitForMultipleEvents(nEvents, events, FALSE, WSA_INFINITE, FALSE);
		if (index == WSA_WAIT_FAILED) {
			printf("Error wait for events %d\n", WSAGetLastError());
			break;
		}
		index = index - WSA_WAIT_EVENT_0;

		for (int i = index; i < nEvents; i++) {
			index = WSAWaitForMultipleEvents(1, &events[i], FALSE, 500, FALSE);

			if (index != WSA_WAIT_FAILED && index != WSA_WAIT_TIMEOUT) {
				WSAEnumNetworkEvents(clients[i].client_fd, events[i], &sockEvent);

				if (sockEvent.lNetworkEvents & FD_ACCEPT) {
					if (sockEvent.iErrorCode[FD_ACCEPT_BIT] != 0) {
						printf("FD_ACCEPT failed with error %d\n", sockEvent.iErrorCode[FD_ACCEPT_BIT]);
						break;
					}

					acceptSock = accept(clients[i].client_fd, NULL, NULL);
					printf("Accept connection from %d\n", acceptSock);

					if (nEvents > WSA_MAXIMUM_WAIT_EVENTS) {
						printf("too many conection!\n");
						closesocket(acceptSock);
						break;
					}

					newEvent = WSACreateEvent();
					WSAEventSelect(acceptSock, newEvent, FD_READ | FD_WRITE | FD_CLOSE);
					for (int i = 0; i <= nEvents; i++)
					{
 						if (clients[i].is_active == 0) {
							events[i] = newEvent;
							clients[i].client_fd = acceptSock;
							clients[i].is_active = 1;
							nEvents++;
							printf("socket %d connected", clients[i].client_fd);
							break;
						}
					}
				} // end of if accept

				if (sockEvent.lNetworkEvents & FD_READ) {
					//read socket
					if (sockEvent.iErrorCode[FD_READ_BIT] != 0) {
						printf("FD_READ failed with error %d\n", sockEvent.iErrorCode[FD_READ_BIT]);
						continue;
					}

					
					if (receive_data(clients[i].client_fd, clients[i].client_buffer, BUFFSIZE, 0) < 0)
						break;
					process_data(&clients[i]);
					if(send_data(clients[i].client_fd, clients[i].client_buffer, BUFFSIZE, 0) < 0)
						break;
				}
				if (sockEvent.lNetworkEvents & FD_WRITE) {
					if (sockEvent.iErrorCode[FD_WRITE_BIT] != 0) {
						printf("FD_WRITE failed with error %d\n", sockEvent.iErrorCode[FD_WRITE_BIT]);
						continue;
					}
				}

				if (sockEvent.lNetworkEvents & FD_CLOSE) {
					//socket closed
					if (sockEvent.iErrorCode[FD_CLOSE_BIT] != 0)
					{
						printf("FD_CLOSE failed with error %d\n", sockEvent.iErrorCode[FD_CLOSE_BIT]);						
						break;
					}

					closesocket(clients[i].client_fd);
					// Remove socket and associated event from
					memset(&clients[i], 0, sizeof(struct client_info_struct));
					nEvents--;
				}

				//reset
				WSAResetEvent(events[i]);
			}//end of test events

		}//end for
	}//end while
}


