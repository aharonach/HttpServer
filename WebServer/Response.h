#include <iostream>
#include "StatusCodes.h"
#include <map>

using namespace std;

class Response
{
private:
	//properties

	float httpVersion;
	string reasonPhrase;
	int statusCode;
	map<string, string> headers;
	string body;

	string fullReasponseString;

public:
	
	//C'tor
	Response();
	
	//D'tor
	~Response();
	
	//Getters
	float getHttpVersion() const;
	string getReasonPhrase() const;
	int getStatusCode() const;
	map<string, string> getHeaders() const;
	string getBody() const;

	//Setters
	void setHttpVersion(float version);
	void setReasonPhrase(string reasonPhrase);
	void setStatusCode(int statusCode);
	void setHeaderInMap(string key, string value);
	void setBody(string body);

	string createReponseString();

private:


};

