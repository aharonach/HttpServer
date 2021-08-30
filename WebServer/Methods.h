#pragma once

#include <string>

using namespace std;


enum class eMethod
{
	HTTP_GET = 1,
	HTTP_POST,
	HTTP_PUT,
	HTTP_DELETE,
	HTTP_HEAD,
	HTTP_OPTIONS,
	HTTP_TRACE
};

eMethod parseMethod(const string& method);
