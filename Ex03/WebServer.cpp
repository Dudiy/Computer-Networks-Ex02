#include "WebServer.h"

void main()
{
	// Initialize Winsock (Windows Sockets).

	// Create a WSADATA object called wsaData.
	// The WSADATA structure contains information about the Windows 
	// Sockets implementation.
	WSAData wsaData;

	// Call WSAStartup and return its value as an integer and check for errors.
	// The WSAStartup function initiates the use of WS2_32.DLL by a process.
	// First parameter is the version number 2.2.
	// The WSACleanup function destructs the use of WS2_32.DLL by a process.
	if (NO_ERROR != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		cout << "Web Server: Error at WSAStartup()\n";
		return;
	}

	// Server side:
	// Create and bind a socket to an internet address.
	// Listen through the socket for incoming connections.

	// After initialization, a SOCKET object is ready to be instantiated.

	// Create a SOCKET object called listenSocket. 
	// For this application:	use the Internet address family (AF_INET), 
	//							streaming sockets (SOCK_STREAM), 
	//							and the TCP/IP protocol (IPPROTO_TCP).
	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// Check for errors to ensure that the socket is a valid socket.
	// Error detection is a key part of successful networking code. 
	// If the socket call fails, it returns INVALID_SOCKET. 
	// The if statement in the previous code is used to catch any errors that
	// may have occurred while creating the socket. WSAGetLastError returns an 
	// error number associated with the last error that occurred.
	if (INVALID_SOCKET == listenSocket)
	{
		cout << "Web Server: Error at socket(): " << WSAGetLastError() << endl;
		WSACleanup();
		return;
	}

	// For a server to communicate on a network, it must bind the socket to 
	// a network address.

	// Need to assemble the required data for connection in sockaddr structure.

	// Create a sockaddr_in object called serverService. 
	sockaddr_in serverService;
	// Address family (must be AF_INET - Internet address family).
	serverService.sin_family = AF_INET;
	// IP address. The sin_addr is a union (s_addr is a unsigned long 
	// (4 bytes) data type).
	// inet_addr (Iternet address) is used to convert a string (char *) 
	// into unsigned long.
	// The IP address is INADDR_ANY to accept connections on all interfaces.
	serverService.sin_addr.s_addr = INADDR_ANY;
	// IP Port. The htons (host to network - short) function converts an
	// unsigned short from host to TCP/IP network byte order 
	// (which is big-endian).
	serverService.sin_port = htons(WEB_SERVER_PORT);

	// Bind the socket for client's requests.

	// The bind function establishes a connection to a specified socket.
	// The function uses the socket handler, the sockaddr structure (which
	// defines properties of the desired connection) and the length of the
	// sockaddr structure (in bytes).
	if (SOCKET_ERROR == bind(listenSocket, (SOCKADDR *)&serverService, sizeof(serverService)))
	{
		cout << "Web Server: Error at bind(): " << WSAGetLastError() << endl;
		closesocket(listenSocket);
		WSACleanup();
		return;
	}

	// Listen on the Socket for incoming connections.
	// This socket accepts only one connection (no pending connections 
	// from other clients). This sets the backlog parameter.
	if (SOCKET_ERROR == listen(listenSocket, 5))
	{
		cout << "Web Server: Error at listen(): " << WSAGetLastError() << endl;
		closesocket(listenSocket);
		WSACleanup();
		return;
	}
	addSocket(listenSocket, LISTEN);

	// Accept connections and handles them one by one.
	while (true)
	{
		// The select function determines the status of one or more sockets,
		// waiting if necessary, to perform asynchronous I/O. Use fd_sets for
		// sets of handles for reading, writing and exceptions. select gets "timeout" for waiting
		// and still performing other operations (Use NULL for blocking). Finally,
		// select returns the number of descriptors which are ready for use (use FD_ISSET
		// macro to check which descriptor in each set is ready to be used).
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

		//
		// Wait for interesting event.
		// Note: First argument is ignored. The fourth is for exceptions.
		// And as written above the last is a timeout, hence we are blocked if nothing happens.
		//
		int nfd;
		nfd = select(0, &waitRecv, &waitSend, NULL, NULL);
		if (nfd == SOCKET_ERROR)
		{
			cout << "Web Server: Error at select(): " << WSAGetLastError() << endl;
			WSACleanup();
			return;
		}

		for (int i = 0; i < MAX_SOCKETS && nfd > 0; i++)
		{
			if (FD_ISSET(sockets[i].id, &waitRecv))
			{
				nfd--;
				switch (sockets[i].recv)
				{
				case LISTEN:
					acceptConnection(i);
					break;

				case RECEIVE:
					receiveMessage(i);
					break;
				}
			}
		}

		for (int i = 0; i < MAX_SOCKETS && nfd > 0; i++)
		{
			if (FD_ISSET(sockets[i].id, &waitSend))
			{
				// TODO change to if
				nfd--;
				if (sockets[i].send == SEND) {
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
			//
			// Set the socket to be in non-blocking mode.
			//
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
}

void acceptConnection(int index)
{
	SOCKET id = sockets[index].id;
	struct sockaddr_in from;		// Address of sending partner
	int fromLen = sizeof(from);

	SOCKET msgSocket = accept(id, (struct sockaddr *)&from, &fromLen);
	if (INVALID_SOCKET == msgSocket)
	{
		cout << "Web Server: Error at accept(): " << WSAGetLastError() << endl;
		return;
	}
	cout << "Web Server: Client " << inet_ntoa(from.sin_addr) << ":" << ntohs(from.sin_port) << " is connected." << endl;


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

	int len = sockets[index].len;
	int bytesRecv = recv(msgSocket, &sockets[index].buffer[len], sizeof(sockets[index].buffer) - len, 0);

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
		return;
	}
	else
	{
		sockets[index].buffer[len + bytesRecv] = '\0'; //add the null-terminating to make it a string
		cout << "Web Server: Recieved: " << bytesRecv << " bytes of \"" << &sockets[index].buffer[len] << "\" message.\n";

		sockets[index].len += bytesRecv;
		HttpRequest request = HttpRequest();


		if (sockets[index].len > 0)
		{
			size_t bytesParsed = request.initFromString(sockets[index].buffer);
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
			/*else if (strncmp(sockets[index].buffer, "Exit", 4) == 0)
			{
				sockets[index].send = IDLE;
				closesocket(msgSocket);
				removeSocket(index);
				return;
			}*/
		}
	}

}

void sendMessage(int index)
{
	int bytesSent = 0;
	char sendBuff[BUFF_SIZE];
	HttpRequest request = sockets[index].request;
	HttpResponse response = HttpResponse();
	SOCKET msgSocket = sockets[index].id;
	time_t timer;
	time(&timer);
	string currDate = ctime(&timer);
	if (sockets[index].sendMethod == GET)
	{
		int dataSize = response.setDataFromFile(request.getURL());
		if (dataSize > 0) {
			response.setOkStatusLine();
		}
		else {
			dataSize = response.setDataNotFound();
			response.setStatusLine(404, "Not found");
		}
		response.addHeaderLine("Content-Length", to_string(dataSize));
		// response.addHeaderLine("Date", currDate);
		// response.addHeaderLine("Content-Type", "text/html");
		// response.addHeaderLine("Connection", "Closed");
		// // Parse the current time to printable string.
		// strcpy(sendBuff, response.toString().c_str());		
	}
	else if (sockets[index].sendMethod == PUT)
	{
		string absoluteFilePath = request.getUrl(); // TODO set absolute path
		bool fileExisted = false;
		ifstream f(name.c_str());
		fileExisted = f.good();		
		bool writeSucceded = writeDataToFile(absoluteFilePath, response);	
		if (!writeSucceded){							
			response.setStatusLine(404, "Not found");	
		} else{			
			if (fileExisted){
				response.setStatusLine(200, "OK");	
			} else {
				response.setStatusLine(201, "Created");				
			}		
			response.addHeaderLine("Content-Location", request.getURL);
		}
	}
	response.addHeaderLine("Date", currDate);
	response.addHeaderLine("Content-Type", "text/html");
	response.addHeaderLine("Connection", "Closed");
	// Parse the current time to printable string.
	strcpy(sendBuff, response.toString().c_str());	

	bytesSent = send(msgSocket, sendBuff, (int)strlen(sendBuff), 0);
	if (SOCKET_ERROR == bytesSent)
	{
		cout << "Time Server: Error at send(): " << WSAGetLastError() << endl;
		return;
	}

	cout << "Time Server: Sent: " << bytesSent << "\\" << strlen(sendBuff) << " bytes of \"" << sendBuff << "\" message.\n";
	// TODO fix bug!
	sockets[index].send = IDLE;
}

