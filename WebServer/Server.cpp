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
#include <algorithm>
#include <time.h>

const int LISTEN_PORT = 8080;
const int TIMEOUT_IN_SECONDS = 120;
const int MAX_SOCKETS = 5;
const int EMPTY = 0;
const int LISTEN = 1;
const int RECEIVE = 2;
const int IDLE = 3;
const int SEND = 4;
static map<int, string> Months = { {1, "Jan"}, {2, "Feb"}, {3, "Mar"}, {4, "Apr"}, {5, "May"}, {6, "Jun"}, {7, "Jul"}, {8, "Aug"}, {9, "Sep"}, {10, "Oct"}, {11, "Nov"}, {12, "Dec"} };
static map<int, string> Days = { {1, "Sun"}, {2, "Mon"}, {3, "Tue"}, {4, "Wed"}, {5, "Thu"}, {6, "Fri"}, {7, "Sat"} };

struct SocketState
{
	SOCKET id = 0;
	int	recv = EMPTY;
	int	send = EMPTY;
	time_t lastTime = 0;
	vector<Request*> requests;
};

bool addSocket(SOCKET id, int what, SocketState* sockets, int* socketsCount);
void removeSocket(int index, SocketState* sockets, int* socketsCount);
void acceptConnection(int index, SocketState* sockets, int* socketsCount);
void receiveMessage(int index, SocketState* sockets, int* socketsCount);
void sendMessage(int index, SocketState* sockets);

void getGETOrHEADResponse(const Request& requestToHandle, Response* response, HTTPFileHandler* fileHandler);
void getPOSTResponse(const Request& requestToHandle, Response* response, HTTPFileHandler* fileHandler);
void getPUTResponse(const Request& requestToHandle, Response* response, HTTPFileHandler* fileHandler);
void getDELETEResponse(const Request& requestToHandle, Response* response, HTTPFileHandler* fileHandler);
void getOPTIONSResponse(const Request& requestToHandle, Response* response, HTTPFileHandler* fileHandler);
void getTRACEResponse(const Request& requestToHandle, Response* response, HTTPFileHandler* fileHandler);
void setResponseTime(Response* response);

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
	serverService.sin_port = htons(LISTEN_PORT);

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

	struct timeval timeOut;
	timeOut.tv_sec = TIMEOUT_IN_SECONDS;
	timeOut.tv_usec = 0;

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
		nfd = select(0, &waitRecv, &waitSend, NULL, &timeOut);
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

		for (int i = 1; i < MAX_SOCKETS; i++)
		{
			if (sockets[i].lastTime == 0)
				continue;

			time_t now;
			time(&now);

			double timeDiffInSeconds = difftime(now, sockets[i].lastTime);

			if (timeDiffInSeconds >= TIMEOUT_IN_SECONDS) {
				cout << "Timeout to client " << sockets[i].id << ". the connection is closed. (" << timeDiffInSeconds << " seconds passed)" << endl;
				closesocket(sockets[i].id);
				removeSocket(i, sockets, &socketsCount);
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
	sockets[index].lastTime = 0;
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

	cout << "Client id: " << msgSocket << ", Address: " << inet_ntoa(from.sin_addr) << ":" << ntohs(from.sin_port) << " is connected." << endl;

	unsigned long flag = 1;
	if (ioctlsocket(msgSocket, FIONBIO, &flag) != 0)
	{
		cout << "Error at ioctlsocket(): " << WSAGetLastError() << endl;
	}

	if (addSocket(msgSocket, RECEIVE, sockets, socketsCount) == false)
	{
		cout << "\t\tToo many connections, dropped!\n";
		closesocket(msgSocket);
	}
	return;
}

void receiveMessage(int index, SocketState* sockets, int* socketsCount)
{
	SOCKET msgSocket = sockets[index].id;
	char buffer[2048];

	int bytesRecv = recv(msgSocket, buffer, sizeof(buffer) - 1, 0);

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
		buffer[bytesRecv] = '\0';
		string bufferString(buffer);

		sockets[index].requests.push_back(new Request(bufferString));
		sockets[index].send = SEND;
		time(&(sockets[index].lastTime));
	}
}

void sendMessage(int index, SocketState* sockets)
{
	int bytesSent = 0;

	SOCKET msgSocket = sockets[index].id;
	Response response;
	HTTPFileHandler fileHandler;
	string responseString;
	bool isRequestValid = false;

	Request* requestToHandle = sockets[index].requests.front();
	sockets[index].requests.erase(sockets[index].requests.begin());

	isRequestValid = requestToHandle->isRequestValid();
	setResponseTime(&response);
	response.addHeader(HEADER_SERVER, "Aharon & Shir - HttpServer");

	if (isRequestValid)
	{
		if (requestToHandle->getHttpVersion() != "HTTP/1.1") 
		{
			response.setStatusCode(HTTP_HTTP_Version_Not_Supported);
			response.setReasonPhrase(PHRASE_HTTP_Version_Not_Supported);
			response.addHeader(HEADER_CONTENT_LENGTH, "0");
			response.addHeader(HEADER_CONTENT_TYPE, "text/html");
		}
		else 
		{
			try 
			{
				switch (requestToHandle->getMethod())
				{
				case eMethod::HTTP_GET:
					getGETOrHEADResponse(*requestToHandle, &response, &fileHandler);
					break;

				case eMethod::HTTP_POST:
					getPOSTResponse(*requestToHandle, &response, &fileHandler);
					break;

				case eMethod::HTTP_PUT:
					getPUTResponse(*requestToHandle, &response, &fileHandler);
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
			}
			catch (const runtime_error& re)
			{
				string body = re.what();
				response.setStatusCode(HTTP_Internal_Server_Error);
				response.setReasonPhrase(PHRASE_Internal_Server_Error);
				response.addHeader(HEADER_CONTENT_LENGTH, to_string(body.size()));
				response.addHeader(HEADER_CONTENT_TYPE, "text/plain");
				response.setBody(body);
			}
			catch (const exception& ex)
			{
				string body = ex.what();
				response.setStatusCode(HTTP_Internal_Server_Error);
				response.setReasonPhrase(PHRASE_Internal_Server_Error);
				response.addHeader(HEADER_CONTENT_LENGTH, to_string(body.size()));
				response.addHeader(HEADER_CONTENT_TYPE, "text/plain");
				response.setBody(body);
			}
			catch (...) {
				response.setStatusCode(HTTP_Internal_Server_Error);
				response.setReasonPhrase(PHRASE_Internal_Server_Error);
				response.addHeader(HEADER_CONTENT_LENGTH, "0");
				response.addHeader(HEADER_CONTENT_TYPE, "text/plain");
			}
		}
	}
	else
	{
		response.setStatusCode(HTTP_Bad_Request);
		response.setReasonPhrase(PHRASE_Bad_Request);
		response.addHeader(HEADER_CONTENT_LENGTH, "0");
		response.addHeader(HEADER_CONTENT_TYPE, "text/html");
	}

	responseString = response.createReponseString();

	bytesSent = send(msgSocket, responseString.c_str(), responseString.size(), 0);
	if (SOCKET_ERROR == bytesSent)
	{
		cout << "Error at send(): " << WSAGetLastError() << endl;
		return;
	}

	if (sockets[index].requests.empty()) 
	{
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

void getGETOrHEADResponse(const Request& requestToHandle, Response* response, HTTPFileHandler* fileHandler)
{
	int responseCode;
	eMethod method = requestToHandle.getMethod();
	string file = fileHandler->getFileInStream(&responseCode, requestToHandle);
	response->setStatusCode(responseCode);
	response->addHeader(HEADER_CONTENT_TYPE, "text/html");

	if (responseCode == HTTP_Not_Found)
	{
		response->addHeader(HEADER_CONTENT_LENGTH, "0");
		response->setReasonPhrase(PHRASE_Not_Found);
	}
	else if (responseCode == HTTP_No_Content)
	{
		response->addHeader(HEADER_CONTENT_LENGTH, "0");
		response->setReasonPhrase(PHRASE_No_Content);
	}
	else
	{
		response->setReasonPhrase(PHRASE_OK);
		response->addHeader(HEADER_CONTENT_LENGTH, to_string(file.size()));

		if (requestToHandle.getQueryParam("lang") != "") 
		{
			response->addHeader(HEADER_CONTENT_LANGUAGE, requestToHandle.getQueryParam("lang"));
		}

		if (method == eMethod::HTTP_GET)
		{
			response->setBody(file);
		}
	}
}

void getPOSTResponse(const Request& requestToHandle, Response* response, HTTPFileHandler* fileHandler)
{
	int responseCode = HTTP_OK;
	response->setStatusCode(responseCode);
	response->addHeader(HEADER_CONTENT_TYPE, "text/html");
	response->addHeader(HEADER_CONTENT_LENGTH, "0");
	response->setReasonPhrase(PHRASE_OK);

	string bodyToPrint = requestToHandle.getBody();
	replace(bodyToPrint.begin(), bodyToPrint.end(), '\r', '\n');

	cout << "\n****************************\nSERVER LOG - POST\n****************************\n";
	cout << "" << bodyToPrint;
	cout << "\n****************************\nSERVER LOG - END\n****************************\n";
}

void getPUTResponse(const Request& requestToHandle, Response* response, HTTPFileHandler* fileHandler)
{
	int responseCode;
	eMethod method = requestToHandle.getMethod();

	if (method == eMethod::HTTP_PUT) {
		responseCode = fileHandler->createAndWriteIntoAFileForPUT(requestToHandle, requestToHandle.getBody());
	}
	else {
		responseCode = HTTP_OK;
	}

	response->setStatusCode(responseCode);
	response->addHeader(HEADER_CONTENT_TYPE, "text/html");
	response->addHeader(HEADER_CONTENT_LENGTH, "0");

	switch (responseCode)
	{
	case HTTP_OK:
		response->setReasonPhrase(PHRASE_OK);
		break;

	case HTTP_No_Content:
		response->setReasonPhrase(PHRASE_No_Content);
		break;

	case HTTP_Not_Implemented:
		response->setReasonPhrase(PHRASE_Not_Implemented);
		break;

	case HTTP_Internal_Server_Error:
		response->setReasonPhrase(PHRASE_Internal_Server_Error);
		break;

	case HTTP_Created:
		response->setReasonPhrase(PHRASE_Created);
		break;
	}
}

void getDELETEResponse(const Request& requestToHandle, Response* response, HTTPFileHandler* fileHandler)
{
	int responseCode;
	responseCode = fileHandler->deleteFile(requestToHandle);
	response->setStatusCode(responseCode);
	response->addHeader(HEADER_CONTENT_TYPE, "text/html");
	response->addHeader(HEADER_CONTENT_LENGTH, "0");

	if (responseCode == HTTP_Not_Found)
	{
		response->setReasonPhrase(PHRASE_Not_Found);
	}
	else if (responseCode == HTTP_No_Content)
	{
		response->setReasonPhrase(PHRASE_No_Content);
	}
	else
	{
		response->setReasonPhrase(PHRASE_OK);
	}
}

void getOPTIONSResponse(const Request& requestToHandle, Response* response, HTTPFileHandler* fileHandler)
{
	response->addHeader(HEADER_CONTENT_TYPE, "text/html");
	response->setReasonPhrase(PHRASE_OK);
	response->setStatusCode(HTTP_OK);
	response->addHeader(HEADER_ALLOW, "PUT, POST, GET, DELETE, OPTIONS, HEAD, TRACE");
	response->addHeader(HEADER_CONTENT_LENGTH, "0");
}

void getTRACEResponse(const Request& requestToHandle, Response* response, HTTPFileHandler* fileHandler)
{
	string body = requestToHandle.getRawRequest();

	response->setStatusCode(HTTP_OK);
	response->setReasonPhrase(PHRASE_OK);
	response->addHeader(HEADER_CONTENT_TYPE, "message/http");
	response->addHeader(HEADER_CONTENT_LENGTH, to_string(body.size()));
	response->setBody(body);
}

void setResponseTime(Response* response)
{
	stringstream timeString;
	time_t timer;
	time(&timer);
	tm* structuredTime = gmtime(&timer);
	timeString << Days[structuredTime->tm_wday + 1];
	timeString << ", ";
	timeString << to_string(structuredTime->tm_mday);
	timeString << " ";
	timeString << Months[structuredTime->tm_mon + 1];
	timeString << " ";
	timeString << to_string(structuredTime->tm_year + 1900);
	timeString << " ";
	timeString << to_string(structuredTime->tm_hour);
	timeString << ":";
	timeString << to_string(structuredTime->tm_min);
	timeString << ":";
	timeString << to_string(structuredTime->tm_sec);
	timeString << " GMT";
	response->addHeader(HEADER_DATE, timeString.str());
}