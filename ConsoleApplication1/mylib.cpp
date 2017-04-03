#include "stdafx.h"
#include <string.h>
#include <winsock2.h>
#include "mylib.h"

#define PASSLENGHT 32
#define USERLENGHT 32
#define BUFFSIZE 512
#define MAXTRY 5


#define DEFAULTUSER "username"
#define DEFAULTPASS "password"

enum {
	UNAUTHENTICATED = 0,
	BLOCKED,
	AUTHENTICATING,
	AUTHENTICATED
};

char* CodeReference(int code) {
	char* return_name;
	switch (code)
	{
	case 230:
		return_name = "Login! Service ready\n";
		break;
	case 231:
		return_name = "User logout! Service terminate\n";
		break;
	case 331:
		return_name = "User name is ok! need password\n";
		break;
	case 332:
		return_name = "Need account for login!\n";
		break;
	case 430:
		return_name = "Invalid username or password\n";
		break;
	case 530:
		return_name = "Not login";
		break;
	default:
		return_name = "Error is not reconigzed";
		break;
	}
	return return_name;
}

int GetClientAction(char* action_name) {
	if (strcmp(action_name, "USER") == 0)
		return 1;
	else if (strcmp(action_name, "PASS") == 0)
		return 2;
	else if (strcmp(action_name, "BYE!") == 0)
		return 0;
	else if (strcmp(action_name, "LOUT") == 0)
		return 9;
	else
		return 2205;
}

int receive_data(SOCKET s, char* buffer, int size, int flags) {
	int n = recv(s, buffer, size, flags);
	if (n <= 0)
		printf("Error! cannot receive message.\n");
	else
		buffer[n] = 0;
	return n;
}

int send_data(SOCKET s, char* buffer, int size, int flags) {
	int n = send(s, buffer, size, flags);
	if (n <= 0)
		printf("Error! cannot send message.\n");
	return n;
}

int process_data(struct client_info_struct* client_info) {
	char* buffer = client_info->client_buffer;
	int returnByte;

	char action_code[5];
	char* password;
	char* username;

	//action indentify
	//read actioncode
	strncpy(action_code, buffer, 4);
	action_code[4] = '\0';


	switch (GetClientAction(action_code))
	{
		// actioncode is USER
	case 1:
		username = &client_info->client_buffer[5];
		client_info->attemt_count++;

		//user try login too many 
		if (client_info->state == BLOCKED) {
			snprintf(client_info->client_buffer, BUFFSIZE, "%dUser is blocked", 530, client_info->user_name);
			return 0;
		}

		//user try to login while still on session
		if (client_info->state == AUTHENTICATED) {
			snprintf(client_info->client_buffer, BUFFSIZE, "%dService in user: %s session, to access other account log out first", 230, client_info->user_name);
			return 0;
		}

		//verify username in database
		strncpy(client_info->user_name, username, USERLENGHT);
		if (strncmp(username, DEFAULTUSER, USERLENGHT) == 0) {
			//username request match user
			snprintf(client_info->client_buffer, BUFFSIZE, "%dLog in account: %s! Enter password\n", 331, client_info->user_name);
			client_info->state = AUTHENTICATING;
			return 0;
		}

		//username not match
		snprintf(client_info->client_buffer, BUFFSIZE, "%dLogin error: not found user %s\n", 430, client_info->user_name);
		if (client_info->attemt_count > MAXTRY) {
			client_info->state = BLOCKED;
		}
		return 0;

		//action code is PASS
	case 2:
		password = client_info->client_buffer + 5;
		client_info->attemt_count++;

		//user try login too many 
		if (client_info->state == BLOCKED) {
			snprintf(client_info->client_buffer, BUFFSIZE, "%dUser %s is blocked", 530, client_info->user_name);
			return 0;
		}

		//user send password while still on session
		if (client_info->state == AUTHENTICATED) {
			snprintf(client_info->client_buffer, BUFFSIZE, "%dService ready to user: %s session", 230, client_info->user_name);
			return 0;
		}

		//verify password for user in database
		if (strncmp(password, DEFAULTPASS, PASSLENGHT) == 0 && strncmp(client_info->user_name, DEFAULTUSER, USERLENGHT) == 0) {
			//password match user's account
			snprintf(buffer, BUFFSIZE, "%dLog in %s: Successfull! Service ready\n", 230, client_info->user_name);
			printf("User %s login!\n", client_info->user_name);
			client_info->state = AUTHENTICATED;

			//reset attempcount, set user in session variable
			client_info->attemt_count = 0;
			return 0;
		}

		//password not match
		snprintf(client_info->client_buffer, BUFFSIZE, "%dLogin error: password not match %s\n", 430, client_info->user_name);
		if (client_info->attemt_count > MAXTRY) {
			client_info->state = BLOCKED;
		}
		return 0;

		//user sent terminate service
	case 0:
		printf("Service terminate!\n");
		memset(client_info, 0, sizeof(struct client_info_struct));
		return 0;

		//log out
	case 9:
		//user send logout without being in session
		if (client_info->state != AUTHENTICATED) {
			snprintf(buffer, BUFFSIZE, "%dLogout error: User not login\n", 451);
			return 0;
		}

		//logout user in session
		client_info->state = UNAUTHENTICATED;
		client_info->attemt_count = 0;
		memset(client_info->user_name, 0, USERLENGHT + 1);
		snprintf(buffer, BUFFSIZE, "%dLogout successful\n", 231);
		return 0;

	default:
		strncpy(buffer, "451Command not recognize\n", BUFFSIZE);
		return -1;
	}
}