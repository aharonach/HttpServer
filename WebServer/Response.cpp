#include <iostream>
#include "StatusCodes.h"

using namespace std;

class Response
{
private:
	int statusCode;
	StatusType statusType;
	string body;

public:
	Response();
	~Response();

private:

};

Response::Response()
{
}

Response::~Response()
{
}