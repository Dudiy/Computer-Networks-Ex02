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
const string ROOT_PATH = "D:\\School\\Year3\\Root";

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

const string METHODS = "GET, PUT, DELETE, HEAD, OPTIONS, TRACE";

typedef enum eFileWriteResult {
	FILE_CREATED = 1,
	FILE_MODIFIED = 2,
	WRITE_FAILED = 3
};

struct SocketState
{
	SOCKET id;			// Socket handle
	int	recv;			// Receiving?
	int	send;			// Sending?
	eMethod sendMethod;	// Sending sub-type
	char buffer[BUFF_SIZE];
	HttpRequest request = HttpRequest(ROOT_PATH);
	int len;
};

const int WEB_SERVER_PORT = 8080;
const int MAX_SOCKETS = 60;
//const string ROOT_PATH = "D:\\School\\Year3\\Root";
const string PATH_404 = string("errorPage.html");

bool addSocket(SOCKET id, int what);
void removeSocket(int index);
void acceptConnection(int index);
void receiveMessage(int index);
void sendMessage(int index);

void handleTRACE(HttpRequest &request, HttpResponse &response);

void handleDELETE(HttpRequest &request, HttpResponse &response);

void handlePUT(HttpRequest &request, HttpResponse &response);

void handleGET(HttpRequest &request, HttpResponse &response);

string readStringFromFile(string i_RelativePath);

eFileWriteResult writeStringToFile(string i_RelativePath, string i_ContentToWrite);

string deleteFile(string i_RelativePath);

// TODO remove from global scope
struct SocketState sockets[MAX_SOCKETS] = { 0 };
int socketsCount = 0;