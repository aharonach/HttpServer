#include <iostream>
#include "StatusCodes.h"
#include "Response.h"

using namespace std;
//C'tor
Response::Response()
{
	setHttpVersion("HTTP/1.1");
}

//D'tor
Response::~Response() 
{
	this->headers.clear();
}

//Getters
const string& Response::getHttpVersion() const
{
	return this->httpVersion;
}

const string& Response::getReasonPhrase() const
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
void Response::setHttpVersion(const string& version)
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

void Response::addHeader(const string & key, const string& value)
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
	
	stringCreator << getHttpVersion() << " " << getStatusCode() << " " << getReasonPhrase() << "\r\n";

	for (auto const& header : getHeaders()) {
		stringCreator << header.first << ": " << header.second << "\r\n";
	}
	
	stringCreator << "\r\n" << getBody() << "\r\n";

	return stringCreator.str();
}
