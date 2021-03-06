#pragma once

#include <iostream>
#include "StatusCodes.h"
#include <map>
#include <sstream>


using namespace std;

class Response
{
private:
	//properties

	string httpVersion;
	string reasonPhrase;
	int statusCode;
	map<string, string> headers;
	string body;

public:
	
	//C'tor
	Response();
	
	//D'tor
	~Response();
	
	//Getters
	const string& getHttpVersion() const;
	const string& getReasonPhrase() const;
	int getStatusCode() const;
	map<string, string> getHeaders() const;
	const string& getBody() const;

	//Setters
	void setHttpVersion(const string& version);
	void setReasonPhrase(const string& reasonPhrase);
	void setStatusCode(int statusCode);
	void addHeader(const string& key, const string& value);
	void setBody(const string& body);

	string createReponseString();

};

