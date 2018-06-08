#include "WebServer.h"


void main()
{
	WSAData wsaData;
	if (NO_ERROR != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		cout << "Web Server: Error at WSAStartup()\n";
		return;
	}

	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == listenSocket)
	{
		cout << "Web Server: Error at socket(): " << WSAGetLastError() << endl;
		WSACleanup();
		return;
	}

	sockaddr_in serverService;
	serverService.sin_family = AF_INET;
	serverService.sin_addr.s_addr = INADDR_ANY;
	serverService.sin_port = htons(WEB_SERVER_PORT);
	if (SOCKET_ERROR == bind(listenSocket, (SOCKADDR *)&serverService, sizeof(serverService)))
	{
		cout << "Web Server: Error at bind(): " << WSAGetLastError() << endl;
		closesocket(listenSocket);
		WSACleanup();
		return;
	}

	if (SOCKET_ERROR == listen(listenSocket, 5))
	{
		cout << "Web Server: Error at listen(): " << WSAGetLastError() << endl;
		closesocket(listenSocket);
		WSACleanup();
		return;
	}

	createRootFolder();
	addSocket(listenSocket, LISTEN);
	cout << "Web Server: Listening on port " << WEB_SERVER_PORT << endl;
	while (true)
	{
		fd_set waitRecv;
		FD_ZERO(&waitRecv);
		for (int i = 0; i < MAX_SOCKETS; i++)
		{
			if ((sockets[i].recv == LISTEN) || (sockets[i].recv == RECEIVE))
				FD_SET(sockets[i].id, &waitRecv);
		}

		fd_set waitSend;
		FD_ZERO(&waitSend);
		for (int i = 0; i < MAX_SOCKETS; i++)
		{
			if (sockets[i].send == SEND)
				FD_SET(sockets[i].id, &waitSend);
		}

		const timeval timeout = {SOCKET_TIMEOT, 0};
		int nfd = select(0, &waitRecv, &waitSend, NULL, &timeout);
		if (nfd == SOCKET_ERROR)
		{
			cout << "Web Server: Error at select(): " << WSAGetLastError() << endl;
			WSACleanup();
			return;
		}

		time_t currTime;
		time(&currTime);
		for (int i = 0; i < MAX_SOCKETS; i++)
		{
			if (sockets[i].recv != EMPTY && sockets[i].recv != LISTEN && currTime - sockets[i].lastActiveTimeStamp >
				SOCKET_TIMEOT)
			{
				cout << "Web Server: Socket " << i << " has timed out." << endl;
				removeSocket(i);
			}
		}

		for (int i = 0; i < MAX_SOCKETS && nfd > 0; i++)
		{
			if (FD_ISSET(sockets[i].id, &waitRecv))
			{
				time(&sockets[i].lastActiveTimeStamp);
				nfd--;
				switch (sockets[i].recv)
				{
				case LISTEN:
					acceptConnection(i);
					break;

				case RECEIVE:
					receiveMessage(i);
					break;
				default:
					break;
				}
			}
		}

		for (int i = 0; i < MAX_SOCKETS && nfd > 0; i++)
		{
			if (FD_ISSET(sockets[i].id, &waitSend))
			{
				time(&sockets[i].lastActiveTimeStamp);
				nfd--;
				if (sockets[i].send == SEND)
				{
					sendMessage(i);
				}
			}
		}
	}

	// Closing connections and Winsock.
	cout << "Web Server: Closing Connection.\n";
	closesocket(listenSocket);
	WSACleanup();
}

bool addSocket(SOCKET id, int what)
{
	for (int i = 0; i < MAX_SOCKETS; i++)
	{
		if (sockets[i].recv == EMPTY)
		{
			sockets[i].id = id;
			sockets[i].recv = what;
			sockets[i].send = IDLE;
			sockets[i].len = 0;
			time(&sockets[i].lastActiveTimeStamp);
			// Set the socket to be in non-blocking mode.			
			unsigned long flag = 1;
			if (ioctlsocket(id, FIONBIO, &flag) != 0)
			{
				cout << "Web Server: Error at ioctlsocket(): " << WSAGetLastError() << endl;
			}
			socketsCount++;
			return (true);
		}
	}
	return (false);
}

void removeSocket(int index)
{
	sockets[index].recv = EMPTY;
	sockets[index].send = EMPTY;
	socketsCount--;
	cout << "Web Server: Socket " << index << " has been removed." << endl;
}

void acceptConnection(int index)
{
	SOCKET id = sockets[index].id;
	struct sockaddr_in from; // Address of sending partner
	int fromLen = sizeof(from);

	SOCKET msgSocket = accept(id, (struct sockaddr *)&from, &fromLen);
	if (INVALID_SOCKET == msgSocket)
	{
		cout << "Web Server: Error at accept(): " << WSAGetLastError() << endl;
		return;
	}
	cout << "Web Server: Client " << inet_ntoa(from.sin_addr) << ":" << ntohs(from.sin_port) <<
		" is connected to socket " << index << "." <<
		endl;


	if (addSocket(msgSocket, RECEIVE) == false)
	{
		cout << "\t\tToo many connections, dropped!\n";
		closesocket(id);
	}
	return;
}

void receiveMessage(int index)
{
	SOCKET msgSocket = sockets[index].id;

	const int len = sockets[index].len;
	const int bytesRecv = recv(msgSocket, &sockets[index].buffer[len], sizeof(sockets[index].buffer) - len, 0);

	if (SOCKET_ERROR == bytesRecv)
	{
		cout << "Web Server: Error at recv(): " << WSAGetLastError() << endl;
		closesocket(msgSocket);
		removeSocket(index);
		return;
	}
	if (bytesRecv == 0)
	{
		closesocket(msgSocket);
		removeSocket(index);
	}
	else
	{
		sockets[index].buffer[len + bytesRecv] = '\0'; //add the null-terminating to make it a string
		cout << "Web Server: Recieved: " << bytesRecv << " bytes of \"" << &sockets[index].buffer[len] <<
			"\" message.\n";

		sockets[index].len += bytesRecv;
		HttpRequest request = HttpRequest(ROOT_PATH);


		if (sockets[index].len > 0)
		{
			const size_t bytesParsed = request.initFromString(sockets[index].buffer);
			memcpy(sockets[index].buffer, &sockets[index].buffer[bytesParsed], sockets[index].len - bytesParsed);
			sockets[index].len -= bytesParsed;
			sockets[index].send = SEND;
			sockets[index].request = request;
			if (!request.getMethod().compare("GET"))
				sockets[index].sendMethod = GET;
			else if (!request.getMethod().compare("PUT"))
				sockets[index].sendMethod = PUT;
			else if (!request.getMethod().compare("DELETE"))
				sockets[index].sendMethod = _DELETE;
			else if (!request.getMethod().compare("HEAD"))
				sockets[index].sendMethod = HEAD;
			else if (!request.getMethod().compare("OPTIONS"))
				sockets[index].sendMethod = OPTIONS;
			else if (!request.getMethod().compare("TRACE"))
				sockets[index].sendMethod = TRACE;
		}
	}
}

void sendMessage(int index)
{
	char sendBuff[BUFF_SIZE];
	HttpRequest request = sockets[index].request;
	HttpResponse response = HttpResponse(ROOT_PATH);
	const SOCKET msgSocket = sockets[index].id;
	time_t timer;
	time(&timer);
	const string currDate = ctime(&timer);
	switch (sockets[index].sendMethod)
	{
	case GET:
		handleGET(request, response);
		break;
	case PUT:
		handlePUT(request, response);
		break;
	case _DELETE:
		handleDELETE(request, response);
		break;
	case HEAD:
		handleGET(request, response);
		if (response.getStatusCode().compare("404"))
			response.setData("");
		break;
	case OPTIONS:
		response.setOkStatusLine();
		response.addHeaderLine("ALLOW", METHODS);
		response.addHeaderLine("Content-Length", "0");
		break;
	case TRACE:
		handleTRACE(request, response);
		break;
	default:
		break;
	}

	response.addHeaderLine("Server", "Dudi's amazing server");
	response.addHeaderLine("Date", currDate);
	response.addHeaderLine("Connection", "Closed");
	strcpy(sendBuff, response.toString().c_str());

	int bytesSent = send(msgSocket, sendBuff, (int)strlen(sendBuff), 0);
	if (SOCKET_ERROR == bytesSent)
	{
		cout << "Web Server: Error at send(): " << WSAGetLastError() << endl;
		return;
	}

	cout << "Web Server: Sent: " << bytesSent << "\\" << strlen(sendBuff) << " bytes of \"" << sendBuff <<
		"\" message.\n";

	// set state to idle only if the buffer is now empty
	if (sockets[index].len == 0 || strlen(sockets[index].buffer) == 0)
	{
		sockets[index].send = IDLE;
	}
}

void handleGET(HttpRequest& i_Request, HttpResponse& i_Response)
{
	string data = readStringFromFile(i_Request.getURL());
	if (data.length() > 0)
	{
		i_Response.setData(data);
		i_Response.setOkStatusLine();
		i_Response.addHeaderLine("Content-Length", to_string(data.length()));
	}
	else
	{
		i_Response.setStatusLine(404, "Not found");
	}
	i_Response.addHeaderLine("Content-Type", "text/html");
}

void handlePUT(HttpRequest& i_Request, HttpResponse& i_Response)
{
	string responseBody = "File was created";
	const eFileWriteResult writeResult = writeStringToFile(i_Request.getURL(), i_Request.getBody());
	switch (writeResult)
	{
	case FILE_CREATED:
		i_Response.setStatusLine(201, "Created");
		break;
	case FILE_MODIFIED:
		i_Response.setOkStatusLine();
		break;
	default:
		set404Response(i_Response);
		break;
	}

	if (writeResult != WRITE_FAILED)
	{
		i_Response.setData(responseBody);
		i_Response.addHeaderLine("Content-Length", to_string(responseBody.length()));
		i_Response.addHeaderLine("Content-Location", i_Request.getURL());
		i_Response.addHeaderLine("Content-Type", "text");
	}
}

void handleTRACE(HttpRequest& i_Request, HttpResponse& i_Response)
{
	string requestStr = i_Request.toString();
	i_Response.setOkStatusLine();
	i_Response.setData(requestStr);
	i_Response.addHeaderLine("Content-Length", to_string(requestStr.length()));
	i_Response.addHeaderLine("Content-Type", "message/http");
}

void handleDELETE(HttpRequest& i_Request, HttpResponse& i_Response)
{
	string fileDeletedContents = deleteFile(i_Request.getURL());
	if (fileDeletedContents.length() > 0)
	{
		i_Response.setStatusLine(200, "OK");
		i_Response.setData(fileDeletedContents);
		i_Response.addHeaderLine("Content-Length", to_string(fileDeletedContents.length()));
		i_Response.addHeaderLine("Content-Type", "text/html");
	}
	else
	{
		set404Response(i_Response);
	}
}

void set404Response(HttpResponse& i_Response)
{
	i_Response.setStatusLine(404, "Not found");
	string responseData404 = readStringFromFile(PATH_404);
	//if 404 file is not found in root
	if (responseData404.length() == 0)
		responseData404 = htmlString_404;

	i_Response.addHeaderLine("Content-Length", to_string(responseData404.length()));
	i_Response.setData(responseData404);
	i_Response.addHeaderLine("Content-Type", "text/html");
}

string convertRelPathToRootPath(string i_RelativePath)
{
	int pos;
	if (!i_RelativePath.compare("/"))
	{
		i_RelativePath = "/index.html";
	}
	while ((pos = i_RelativePath.find('/')) != string::npos)
	{
		i_RelativePath.replace(pos, 1, "\\");
	}
	return ROOT_PATH + i_RelativePath;
}

string readStringFromFile(const string i_RelativePath)
{
	ifstream file;
	const string absPath = convertRelPathToRootPath(i_RelativePath);
	string fileContents = "";
	string line;

	file.open(absPath);
	if (file.is_open())
	{
		while (getline(file, line))
		{
			fileContents.append(line);
		}
	}

	file.close();
	return fileContents;
}

eFileWriteResult writeStringToFile(const string i_RelativePath, string i_ContentToWrite)
{
	ofstream file;
	const string filePath = convertRelPathToRootPath(i_RelativePath);
	string line;
	eFileWriteResult writeResult;
	try
	{
		writeResult = experimental::filesystem::exists(filePath) ? FILE_MODIFIED : FILE_CREATED;
		file.open(filePath);
		if (!file.is_open())
			throw exception("failed to open file");

		file.write(i_ContentToWrite.c_str(), i_ContentToWrite.length());
	}
	catch (exception e)
	{
		writeResult = WRITE_FAILED;
	}

	file.close();
	return writeResult;
}

string deleteFile(const string i_RelativePath)
{
	string fileContents = readStringFromFile(i_RelativePath);
	string absPath = convertRelPathToRootPath(i_RelativePath);
	remove(absPath.c_str());
	return fileContents;
}

void createRootFolder()
{
	if (CreateDirectory(ROOT_PATH.c_str(), NULL))
	{
		writeStringToFile("/index.html", htmlString_index);
		writeStringToFile("/errorPage.html", htmlString_404);
		cout << "Web Server: Created root folder with index.html and errorPage.html" << endl;
	}
}
