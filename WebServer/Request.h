#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
#include "Methods.h"

using namespace std;

class Request
{
private:
	// Properties
	string rawReqeust;

	eMethod method;
	string path;
	unordered_map<string, string> queryParams;
	unordered_map<string, string> headers;
	string body;
	float httpVersion;

	// Private setters, so the request could not be modified outside it.
	void setMethod(eMethod method);
	void setBody(const string& body);
	void setPath(const string& path);
	void addQueryParam(const string& field, const string& value);
	void addHeader(const string& field, const string& value);
	void setHttpVersion(float version);

	// Parse the request
	void parseRequest();

public:
	// C'tor
	Request(const string& rawRequest);

	// D'tor
	~Request();

	// Public getters
	eMethod getMethod() const;
	string getPath() const;
	string getQueryParam(const string& field) const;
	float getHttpVersion() const;
	string getHeader(const string& field) const;
	string getBody() const;
};