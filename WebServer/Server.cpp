#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma warning(disable: 4326 4996)
#include <iostream>
using namespace std;
#pragma comment(lib, "Ws2_32.lib")
#include <winsock2.h>
#include <string.h>
#include <time.h>
#include "Request.h"
#include "Response.h"
#include "Methods.h"
#include "StatusCodes.h"
#include "FileHandler.h"

struct SocketState
{
	SOCKET id;			// Socket handle
	int	recv;			// Receiving?
	int	send;			// Sending?
	int sendSubType;	// Sending sub-type
	char buffer[1024];
	vector<Request*> requests;
	int len;
};

const int TIME_PORT = 27015;
const int MAX_SOCKETS = 5;
const int EMPTY = 0;
const int LISTEN = 1;
const int RECEIVE = 2;
const int IDLE = 3;
const int SEND = 4;

bool addSocket(SOCKET id, int what, SocketState* sockets, int* socketsCount);
void removeSocket(int index, SocketState* sockets, int* socketsCount);
void acceptConnection(int index, SocketState* sockets, int* socketsCount);
void receiveMessage(int index, SocketState* sockets, int* socketsCount);
void sendMessage(int index, SocketState* sockets);




void main()
{
	WSAData wsaData;
	struct SocketState sockets[MAX_SOCKETS] = { 0 };
	int socketsCount = 0;

	if (NO_ERROR != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		cout << "Time Server: Error at WSAStartup()\n";
		return;
	}

	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (INVALID_SOCKET == listenSocket)
	{
		cout << "Time Server: Error at socket(): " << WSAGetLastError() << endl;
		WSACleanup();
		return;
	}

	sockaddr_in serverService;
	serverService.sin_family = AF_INET;
	serverService.sin_addr.s_addr = INADDR_ANY;
	serverService.sin_port = htons(TIME_PORT);

	if (SOCKET_ERROR == bind(listenSocket, (SOCKADDR*)&serverService, sizeof(serverService)))
	{
		cout << "Time Server: Error at bind(): " << WSAGetLastError() << endl;
		closesocket(listenSocket);
		WSACleanup();
		return;
	}

	if (SOCKET_ERROR == listen(listenSocket, 5))
	{
		cout << "Time Server: Error at listen(): " << WSAGetLastError() << endl;
		closesocket(listenSocket);
		WSACleanup();
		return;
	}
	addSocket(listenSocket, LISTEN, sockets, &socketsCount);

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

		int nfd;
		nfd = select(0, &waitRecv, &waitSend, NULL, NULL);
		if (nfd == SOCKET_ERROR)
		{
			cout << "Time Server: Error at select(): " << WSAGetLastError() << endl;
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
					acceptConnection(i, sockets, &socketsCount);
					break;

				case RECEIVE:
					receiveMessage(i, sockets, &socketsCount);
					break;
				}
			}
		}

		for (int i = 0; i < MAX_SOCKETS && nfd > 0; i++)
		{
			if (FD_ISSET(sockets[i].id, &waitSend))
			{
				nfd--;
				switch (sockets[i].send)
				{
				case SEND:
					sendMessage(i, sockets);
					break;
				}
			}
		}
	}

	cout << "Time Server: Closing Connection.\n";
	closesocket(listenSocket);
	WSACleanup();
}

bool addSocket(SOCKET id, int what, SocketState* sockets, int* socketsCount)
{
	for (int i = 0; i < MAX_SOCKETS; i++)
	{
		if (sockets[i].recv == EMPTY)
		{
			sockets[i].id = id;
			sockets[i].recv = what;
			sockets[i].send = IDLE;
			sockets[i].len = 0;
			*socketsCount++;
			return (true);
		}
	}
	return (false);
}

void removeSocket(int index, SocketState* sockets, int* socketsCount)
{
	sockets[index].recv = EMPTY;
	sockets[index].send = EMPTY;
	*socketsCount--;
}

void acceptConnection(int index, SocketState* sockets, int* socketsCount)
{
	SOCKET id = sockets[index].id;
	struct sockaddr_in from;
	int fromLen = sizeof(from);

	SOCKET msgSocket = accept(id, (struct sockaddr*)&from, &fromLen);
	if (INVALID_SOCKET == msgSocket)
	{
		cout << "Time Server: Error at accept(): " << WSAGetLastError() << endl;
		return;
	}
	cout << "Time Server: Client " << inet_ntoa(from.sin_addr) << ":" << ntohs(from.sin_port) << " is connected." << endl;

	unsigned long flag = 1;
	if (ioctlsocket(msgSocket, FIONBIO, &flag) != 0)
	{
		cout << "Time Server: Error at ioctlsocket(): " << WSAGetLastError() << endl;
	}

	if (addSocket(msgSocket, RECEIVE, sockets, socketsCount) == false)
	{
		cout << "\t\tToo many connections, dropped!\n";
		closesocket(id);
	}
	return;
}

void receiveMessage(int index, SocketState* sockets, int* socketsCount)
{
	SOCKET msgSocket = sockets[index].id;

	int len = sockets[index].len;
	int bytesRecv = recv(msgSocket, &sockets[index].buffer[len], sizeof(sockets[index].buffer) - len, 0);

	if (SOCKET_ERROR == bytesRecv)
	{
		cout << "Time Server: Error at recv(): " << WSAGetLastError() << endl;
		closesocket(msgSocket);
		removeSocket(index, sockets, socketsCount);
		return;
	}
	if (bytesRecv == 0)
	{
		closesocket(msgSocket);
		removeSocket(index, sockets, socketsCount);
		return;
	}
	else
	{
		sockets[index].buffer[len + bytesRecv] = '\0';
		cout << "Time Server: Recieved: " << bytesRecv << " bytes of \"" << &sockets[index].buffer[len] << "\" message.\n";

		sockets[index].len += bytesRecv;
		sockets[index].requests.push_back(new Request(sockets[index].buffer));
		memset(sockets[index].buffer, 0, sizeof(sockets[index].buffer));
		sockets[index].send = SEND;
	}
}

void sendMessage(int index, SocketState* sockets)
{
	int bytesSent = 0;

	SOCKET msgSocket = sockets[index].id;
	Response response;
	HTTPFileHandler fileHandler;
	int responseCode;
	string responseString;
	
	//check request type
	//action by request
	//create response
	
	Request* requestToHandle = sockets[index].requests.front();


	switch (requestToHandle->getMethod())
	{
	case eMethod::HTTP_GET:
	{
		string file = fileHandler.getFileInStream(requestToHandle->getPath(),&responseCode);
		if (responseCode == HTTP_Not_Found) 
		{
			response.setStatusCode(HTTP_Not_Found);
			response.setHeaderInMap(CONTENT_LENGTH, "0");
			response.setReasonPhrase("Not Found");
		}
		else if (responseCode == HTTP_No_Content)
		{
			response.setStatusCode(HTTP_No_Content);
			response.setHeaderInMap(CONTENT_LENGTH, "0");
			response.setReasonPhrase("No Content");
		}
		else 
		{
			response.setStatusCode(HTTP_OK);
			response.setHeaderInMap(CONTENT_LENGTH, to_string(file.size()));
			response.setReasonPhrase("ok");
			response.setBody(file);
		}
		break;
	}
	case eMethod::HTTP_POST:
	{
		break;
	}
	case eMethod::HTTP_PUT:
	{
		break;
	}
	case eMethod::HTTP_DELETE:
	{
		break;
	}
	case eMethod::HTTP_HEAD:
	{
		break;
	}
	case eMethod::HTTP_OPTIONS:
	{
		break;
	}
	case eMethod::HTTP_TRACE:
	{
		break;
	}

	default:
		break;
	}
	
	responseString = response.createReponseString();

	bytesSent = send(msgSocket, responseString.c_str(), responseString.size(), 0);
	if (SOCKET_ERROR == bytesSent)
	{
		cout << "Time Server: Error at send(): " << WSAGetLastError() << endl;
		return;
	}

	cout << "Time Server: Sent: " << bytesSent << "\\" << responseString.size() << " bytes of \"" << responseString << "\" message.\n";

	sockets[index].send = IDLE;

}

eMethod parseMethod(const string& method)
{
	if ("GET" == method)
		return eMethod::HTTP_GET;

	else if ("POST" == method)
		return eMethod::HTTP_POST;

	else if ("PUT" == method)
		return eMethod::HTTP_PUT;

	else if ("DELETE" == method)
		return eMethod::HTTP_DELETE;

	else if ("HEAD" == method)
		return eMethod::HTTP_HEAD;

	else if ("OPTIONS" == method)
		return eMethod::HTTP_OPTIONS;

	else
		return eMethod::HTTP_TRACE;
}