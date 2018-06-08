#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <string.h>
#include <time.h>
#include <fstream>
#include <winsock2.h>
#include <filesystem>
#include "HttpRequest.h"
#include "HttpResponse.h"
using namespace std;

#define BUFF_SIZE 1024
const string ROOT_PATH = "root";

enum eStatus {
	EMPTY = 0,
	LISTEN = 1,
	RECEIVE = 2,
	IDLE = 3,
	SEND = 4
};

enum eMethod {
	GET = 1,
	PUT = 2,
	_DELETE = 3,
	HEAD = 4,
	OPTIONS = 5,
	TRACE = 6
};

const string METHODS = "GET, PUT, DELETE, HEAD, OPTIONS, TRACE";

enum eFileWriteResult {
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
	time_t lastActiveTimeStamp;
	HttpRequest request = HttpRequest(ROOT_PATH);
	int len;
};

const int WEB_SERVER_PORT = 8080;
const int SOCKET_TIMEOT = 120;
const int MAX_SOCKETS = 60;
const string PATH_404 = string("/errorPage.html");
const string htmlString_404 = string("<html><head><title>404 Not Found</title></head><body><h1>Not Found</h1><p>The requested URL was not found on this server.</p></body></html>");
const string htmlString_index = string("<html><head><title>Hello</title></head><body><h1>Hello world :)</h1></body></html>");

void createRootFolder();
bool addSocket(SOCKET id, int what);
void removeSocket(int index);
void acceptConnection(int index);
void receiveMessage(int index);
void sendMessage(int index);
void handleGET(HttpRequest &i_Request, HttpResponse &i_Response);
void handlePUT(HttpRequest &i_Request, HttpResponse &i_Response);
void handleDELETE(HttpRequest &i_Request, HttpResponse &i_Response);
void handleTRACE(HttpRequest &i_Request, HttpResponse &i_Response);
void set404Response(HttpResponse & i_Response);
string readStringFromFile(string i_RelativePath);
eFileWriteResult writeStringToFile(string i_RelativePath, string i_ContentToWrite);
string deleteFile(string i_RelativePath);
struct SocketState sockets[MAX_SOCKETS] = { 0 };
int socketsCount = 0;