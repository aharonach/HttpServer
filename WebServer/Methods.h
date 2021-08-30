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

eMethod parseMethod(const string & method)
{
	if ("GET" == method)
		return eMethod::HTTP_GET;

	else if ("POST" == method)
		return eMethod::HTTP_POST;

	else if ("PUT" == method)
		return eMethod::HTTP_PUT;

	else if ("DELETE" == method)
		return eMethod::HTTP_DELETE;

	else if ("HEAD" == method)
		return eMethod::HTTP_HEAD;

	else if ("OPTIONS" == method)
		return eMethod::HTTP_OPTIONS;

	else
		return eMethod::HTTP_TRACE;
}