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
#include "HTTPFileHandler.h"

struct SocketState
{
	SOCKET id;			// Socket handle
	int	recv;			// Receiving?
	int	send;			// Sending?
	int sendSubType;	// Sending sub-type
	char buffer[1024];
	vector<Request*> requests;
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
		cout << "Error at WSAStartup()\n";
		return;
	}

	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (INVALID_SOCKET == listenSocket)
	{
		cout << "Error at socket(): " << WSAGetLastError() << endl;
		WSACleanup();
		return;
	}

	sockaddr_in serverService;
	serverService.sin_family = AF_INET;
	serverService.sin_addr.s_addr = INADDR_ANY;
	serverService.sin_port = htons(TIME_PORT);

	if (SOCKET_ERROR == bind(listenSocket, (SOCKADDR*)&serverService, sizeof(serverService)))
	{
		cout << "Error at bind(): " << WSAGetLastError() << endl;
		closesocket(listenSocket);
		WSACleanup();
		return;
	}

	if (SOCKET_ERROR == listen(listenSocket, 5))
	{
		cout << "Error at listen(): " << WSAGetLastError() << endl;
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
			cout << "Error at select(): " << WSAGetLastError() << endl;
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

	cout << "Closing Connection.\n";
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
			(*socketsCount)++;
			return true;
		}
	}
	return false;
}

void removeSocket(int index, SocketState* sockets, int* socketsCount)
{
	sockets[index].recv = EMPTY;
	sockets[index].send = EMPTY;
	(*socketsCount)--;
}

void acceptConnection(int index, SocketState* sockets, int* socketsCount)
{
	SOCKET id = sockets[index].id;
	struct sockaddr_in from;
	int fromLen = sizeof(from);

	SOCKET msgSocket = accept(id, (struct sockaddr*)&from, &fromLen);
	if (INVALID_SOCKET == msgSocket)
	{
		cout << "Error at accept(): " << WSAGetLastError() << endl;
		return;
	}
	cout << "Client " << inet_ntoa(from.sin_addr) << ":" << ntohs(from.sin_port) << " is connected." << endl;

	unsigned long flag = 1;
	if (ioctlsocket(msgSocket, FIONBIO, &flag) != 0)
	{
		cout << "Error at ioctlsocket(): " << WSAGetLastError() << endl;
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

	int bytesRecv = recv(msgSocket, sockets[index].buffer, sizeof(sockets[index].buffer) - 1, 0);

	if (SOCKET_ERROR == bytesRecv)
	{
		cout << "Error at recv(): " << WSAGetLastError() << endl;
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
		sockets[index].buffer[bytesRecv] = '\0';
		cout << "Recieved: " << bytesRecv << " bytes of \"" << sockets[index].buffer << "\" message.\n";

		string buffer(sockets[index].buffer);
		memset(sockets[index].buffer, 0, sizeof(sockets[index].buffer));

		sockets[index].requests.push_back(new Request(buffer));
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
	sockets[index].requests.erase(sockets[index].requests.begin());

	switch (requestToHandle->getMethod())
	{
		case eMethod::HTTP_GET:
			getGETOrHEADResponse(*requestToHandle, &response, &fileHandler);
			break;

		case eMethod::HTTP_POST:
			getPUTOrPOSTResponse(*requestToHandle, &response, &fileHandler);
			break;

		case eMethod::HTTP_PUT:
			getPUTOrPOSTResponse(*requestToHandle, &response, &fileHandler);
			break;
	
		case eMethod::HTTP_DELETE:
			getDELETEResponse(*requestToHandle, &response, &fileHandler);
			break;
	
		case eMethod::HTTP_HEAD:
			getGETOrHEADResponse(*requestToHandle, &response, &fileHandler);
			break;
	
		case eMethod::HTTP_OPTIONS:
			getOPTIONSResponse(*requestToHandle, &response, &fileHandler);
			break;
	
		case eMethod::HTTP_TRACE:
			getTRACEResponse(*requestToHandle, &response, &fileHandler);
			break;
	}
	
	responseString = response.createReponseString();

	bytesSent = send(msgSocket, responseString.c_str(), responseString.size(), 0);
	if (SOCKET_ERROR == bytesSent)
	{
		cout << "Error at send(): " << WSAGetLastError() << endl;
		return;
	}

	cout << "Sent: " << bytesSent << "\\" << responseString.size() << " bytes of \"" << responseString << "\" message.\n";

	if (sockets[index].requests.empty()) {
		sockets[index].send = IDLE;
	}
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

void getGETOrHEADResponse(const Request& requestToHandle, Response* response, HTTPFileHandler* fileHandler ) {

	int responseCode;
	eMethod method = requestToHandle.getMethod();
	string file = fileHandler->getFileInStream(requestToHandle.getPath(), &responseCode);
	response->setStatusCode(responseCode);
	response->setHeaderInMap(CONTENT_TYPE, "text/html");

	if (responseCode == HTTP_Not_Found)
	{
		response->setHeaderInMap(CONTENT_LENGTH, "0");
		response->setReasonPhrase("Not Found");
	}
	else if (responseCode == HTTP_No_Content)
	{
		response->setHeaderInMap(CONTENT_LENGTH, "0");
		response->setReasonPhrase("No Content");
	}
	else
	{
		response->setReasonPhrase("OK");
		if (method == eMethod::HTTP_GET)
		{
			response->setHeaderInMap(CONTENT_LENGTH, to_string(file.size()));
			response->setBody(file);
		}
		else
		{
			response->setHeaderInMap(CONTENT_LENGTH, "0");
		}
	}
}

void getPUTOrPOSTResponse(const Request& requestToHandle, Response* response, HTTPFileHandler* fileHandler)
{
	int responseCode;
	eMethod method = requestToHandle.getMethod();
	if (method == eMethod::HTTP_GET) {
		responseCode = fileHandler->createAndWriteIntoAFileForPUT(requestToHandle.getPath(), requestToHandle.getBody());
	}
	else {
		responseCode = fileHandler->createAndWriteIntoAFileForPOST(requestToHandle.getPath(), requestToHandle.getBody());
	}
	
	response->setStatusCode(responseCode);
	response->setHeaderInMap(CONTENT_TYPE, "text/http");
	response->setHeaderInMap(CONTENT_LENGTH, "0");
	switch (responseCode)
	{
	case HTTP_OK:
		response->setReasonPhrase("OK");
		if (method == eMethod::HTTP_POST) {
			cout << requestToHandle.getBody() << "\n";
		}
		break;
	case HTTP_No_Content:
		response->setReasonPhrase("No Content");
		break;
	case HTTP_Not_Implemented:
		response->setReasonPhrase("Not Implemented");
		break;
	case HTTP_Internal_Server_Error:
		response->setReasonPhrase("Internal Server Error");
		break;
	case HTTP_Created:
		response->setReasonPhrase("Created");
		break;
	}
}

void getDELETEResponse(const Request& requestToHandle, Response* response, HTTPFileHandler* fileHandler)
{
	int responseCode;
	responseCode = fileHandler->deleteFile(requestToHandle.getPath());
	response->setStatusCode(responseCode);
	response->setHeaderInMap(CONTENT_TYPE, "text/html");
	response->setHeaderInMap(CONTENT_LANGUAGE, 0);

	if (responseCode == HTTP_Not_Found)
	{
		response->setReasonPhrase("Not Found");
	}
	else if (responseCode == HTTP_No_Content)
	{
		response->setReasonPhrase("No Content");
	}
	else
	{
		response->setReasonPhrase("OK");
	}
}

void getOPTIONSResponse(const Request& requestToHandle, Response* response, HTTPFileHandler* fileHandler)
{
	response->setHeaderInMap(CONTENT_TYPE, "text/html");
	response->setReasonPhrase("OK");
	response->setStatusCode(HTTP_OK);
	response->setHeaderInMap(ALLOW, "Allow: PUT, POST, GET, DELETE, OPTIONS, HEAD, TRACE");
	response->setHeaderInMap(CONTENT_LENGTH, "0");
}

void getTRACEResponse(const Request& requestToHandle, Response* response, HTTPFileHandler* fileHandler)
{
	string body = requestToHandle.getRawRequest();

	response->setStatusCode(HTTP_OK);
	response->setReasonPhrase("OK");
	response->setHeaderInMap(CONTENT_TYPE, "message/http");
	response->setHeaderInMap(CONTENT_LENGTH, to_string(body.size()));
	response->setBody(body);
}
