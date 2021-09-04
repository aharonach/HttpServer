#include "Request.h"

using namespace std;

Request::Request(const string& rawRequest): root("www")
{
	this->rawRequest = rawRequest;
	this->parseRequest();
}

Request::~Request()
{
	this->headers.clear();
	this->queryParams.clear();
}

const bool Request::isRequestValid() const
{
	return !getHttpVersion().empty() && !getPath().empty() && static_cast<underlying_type<eMethod>::type>(getMethod()) > 0;
}

eMethod Request::getMethod() const
{
	return this->method;
}

const string& Request::getPath() const
{
	return this->path;
}

const string& Request::getQueryParam(const string& field) const
{
	try {
		return this->queryParams.at(field);
	}
	catch (...) {
		return "";
	}
}

const string& Request::getHttpVersion() const
{
	return this->httpVersion;
}

const string& Request::getHeader(const string& field) const
{
	return this->headers.at(field);
}

const string& Request::getBody() const
{
	return this->body;
}

const string& Request::getRawRequest() const
{
	return this->rawRequest;
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

void Request::setHttpVersion(const string& version)
{
	this->httpVersion = version;
}

void Request::parseRequest()
{
	stringstream requestStream(this->rawRequest);
	string requestLine, requestBody;

	getline(requestStream, requestLine);

	for (string line; getline(requestStream, line); )
	{
		if (line.find('\r') != string::npos)
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

	for (string line; getline(requestStream, line); )
	{
		requestBody += line;
	}

	stringstream requestLineStream(requestLine);
	vector<string> requestLineArray;

	// Parse the request line (method, path, http version)
	for (string line; getline(requestLineStream, line, ' '); )
	{
		if (line.find('\r') != string::npos)
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

	setHttpVersion(requestLineArray[2]);
	setMethod(parseMethod(requestLineArray[0]));
	setPath(root + path);
	setBody(requestBody);
}