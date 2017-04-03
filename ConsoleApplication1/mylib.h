#define PASSLENGHT 32
#define USERLENGHT 32
#define BUFFSIZE 512

struct client_info_struct {
	char user_name[USERLENGHT];
	int state;
	int attemt_count;
	SOCKET client_fd;
	char client_buffer[BUFFSIZE];
	int is_active;
};

//Server respond format [3char: respond code][string: message]
struct ServerRespondHeader {
	char code[3];
};

//Client send format [3char: message identify type][string: message]
struct ClientRequestHeader {
	char type[4];
};

char* CodeReference(int);
int GetClientAction(char*);
int receive_data(SOCKET s, char* buffer, int size, int flags);
int send_data(SOCKET s, char* buffer, int size, int flags);
int process_data(struct client_info_struct* client_info);