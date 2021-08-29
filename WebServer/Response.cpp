#include <iostream>
#include "StatusCodes.h"
#include "Response.h"

using namespace std;
//C'tor
Response::Response() 
{
}

//D'tor
Response::~Response() 
{
	this->headers.clear();
}

//Getters
float Response::getHttpVersion() const
{
	return this->httpVersion;
}

string Response::getReasonPhrase() const
{
	return this->reasonPhrase;
}

int Response::getStatusCode() const
{
	return this->statusCode
}

map<string, string> Response::getHeaders() const
{
	return this->headers;
}

string Response::getBody() const
{
	return this->body;
}

//Setters
void Response::setHttpVersion(float version)
{
	this->httpVersion = version;
}

void Response::setReasonPhrase(string reasonPhrase)
{
	this->reasonPhrase = reasonPhrase;
}

void Response::setStatusCode(int statusCode)
{
	this->statusCode = statusCode;
}

void Response::setHeaderInMap(string key, string value)
{
	this->headers.insert(key, value);
}

void Response::setBody(string body)
{
	this->body = body;
}

stringstream Response::createReponseString()
{
	stringstream fullResponse;
	
	fullResponse << "HTTP/" << this->httpVersion << " " << this->statusCode << " " << this->reasonPhrase << "\n";	
	for (auto const& header : this->headers) {
		fullResponse << header.first << ": " << header.second << "\n";
	}	
	fullResponse << "\n" << this->body << "\n";

	return fullReasponseString;
}
