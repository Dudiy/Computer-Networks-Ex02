#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
using namespace std;
// Don't forget to include "Ws2_32.lib" in the library list.
#include <winsock2.h>
#include <string.h>
#include <time.h>
#include <fstream>
#include "HttpRequest.h"
#include "HttpResponse.h"

#define BUFF_SIZE 1024

typedef enum eStatus {
	EMPTY = 0,
	LISTEN = 1,
	RECEIVE = 2,
	IDLE = 3,
	SEND = 4
};

typedef enum eMethod {
	GET = 1,
	PUT = 2,
	_DELETE = 3,
	HEAD = 4,
	OPTIONS = 5,
	TRACE = 6
};

struct SocketState
{
	SOCKET id;			// Socket handle
	int	recv;			// Receiving?
	int	send;			// Sending?
	eMethod sendMethod;	// Sending sub-type
	char buffer[BUFF_SIZE];
	HttpRequest request;
	int len;
};


const int WEB_SERVER_PORT = 8080;
const int MAX_SOCKETS = 60;

bool addSocket(SOCKET id, int what);
void removeSocket(int index);
void acceptConnection(int index);
void receiveMessage(int index);
void sendMessage(int index);

// TODO remove from global scope
struct SocketState sockets[MAX_SOCKETS] = { 0 };
int socketsCount = 0;