#include "Request.h"

Request::Request(const string& rawRequest)
{
	this->rawReqeust = rawReqeust;
	this->parseRequest();
}

Request::~Request()
{
	this->headers.clear();
	this->queryParams.clear();
}

eMethod Request::getMethod() const
{
	return this->method;
}

string Request::getPath() const
{
	return this->path;
}

string Request::getQueryParam(const string& field) const
{
	return this->queryParams.at(field);
}

float Request::getHttpVersion() const
{
	return this->httpVersion;
}

string Request::getHeader(const string& field) const
{
	return this->headers.at(field);
}

string Request::getBody() const
{
	return this->body;
}

void Request::setMethod(eMethod method)
{
	this->method = method;
}

void Request::setBody(const string& body)
{
	this->body = body;
}

void Request::setPath(const string& path)
{
	this->path = path;
}

void Request::addQueryParam(const string& field, const string& value)
{
	this->queryParams.insert(make_pair(field, value));
}

void Request::addHeader(const string& field, const string& value)
{
	this->headers.insert(make_pair(field, value));
}

void Request::setHttpVersion(float version)
{
	this->httpVersion = version;
}

void Request::parseRequest()
{
	stringstream requestStream(this->rawReqeust);
	string requestLine, requestBody;

	getline(requestStream, requestLine);

	for (string line; getline(requestStream, line); )
	{
		if (line.find('\n') != string::npos)
		{
			line.erase(line.size() - 1);
		}

		// Got to the body of the request. break the loop!
		if (line.empty())
		{
			break;
		}

		size_t headerSep = line.find(':');
		if (headerSep != string::npos)
		{
			addHeader(line.substr(0, headerSep), line.substr(headerSep + 2));
		}
	}

	getline(requestStream, requestBody);

	stringstream requestLineStream(requestLine);
	vector<string> requestLineArray;

	// Parse the request line (method, path, http version)
	for (string line; getline(requestLineStream, line, ' '); )
	{
		if (line.find('\n') != string::npos)
		{
			line.erase(line.size() - 1);
		}

		requestLineArray.push_back(line);
	}

	stringstream pathStream(requestLineArray[1]);
	string path;
	string queryString;

	getline(pathStream, path, '?');
	getline(pathStream, queryString);

	// Parse query strings if exists
	if (!queryString.empty())
	{
		stringstream queryStringStream(queryString);

		for (string queryParam; getline(queryStringStream, queryParam, '&'); )
		{
			size_t separator = queryParam.find('=');
			if (separator != string::npos)
			{
				addQueryParam(queryParam.substr(0, separator), queryParam.substr(separator + 1));
			}
		}
	}

	float httpVersion = stof(requestLineArray[2].substr(requestLineArray[2].find('/') + 1));

	setHttpVersion(httpVersion);
	setMethod(parseMethod(requestLineArray[0]));
	setPath(path);
	setBody(body);
}