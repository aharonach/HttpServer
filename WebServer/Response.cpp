#include <iostream>
#include "StatusCodes.h"
#include "Response.h"

using namespace std;
//C'tor
Response::Response()
{
	setHttpVersion(1.1f);
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
	return this->statusCode;
}

map<string, string> Response::getHeaders() const
{
	return this->headers;
}

const string& Response::getBody() const
{
	return this->body;
}

//Setters
void Response::setHttpVersion(float version)
{
	this->httpVersion = version;
}

void Response::setReasonPhrase(const string& reasonPhrase)
{
	this->reasonPhrase = reasonPhrase;
}

void Response::setStatusCode(int statusCode)
{
	this->statusCode = statusCode;
}

void Response::setHeaderInMap(const string & key, const string& value)
{
	this->headers.insert(make_pair(key, value));
}

void Response::setBody(const string & body)
{
	this->body = body;
}

string Response::createReponseString()
{
	stringstream stringCreator;
	
	stringCreator << "HTTP/" << this->httpVersion << " " << this->statusCode << " " << this->reasonPhrase << "\r\n";	
	for (auto const& header : this->headers) {
		stringCreator << header.first << ": " << header.second << "\r\n";
	}	
	stringCreator << "\r\n" << this->body << "\r\n";

	return stringCreator.str();
}
