#pragma once

// 200 Codes
#define HTTP_OK 200
#define HTTP_Created 201
#define HTTP_Accepted 202
#define HTTP_No_Content 204

// 400 Codes
#define HTTP_Bad_Request 400
#define HTTP_Not_Found 404

// 500 Codes
#define HTTP_Internal_Server_Error 500
#define HTTP_Not_Implemented 501
#define HTTP_Bad_Gateway 502
#define HTTP_Service_Unavailable 503
#define HTTP_HTTP_Version_Not_Supported 505

//Response headers
#define HEADER_ALLOW "Allow"
#define HEADER_DATE "Date"
#define	HEADER_SERVER "Server"
#define	HEADER_CONTENT_LANGUAGE "Content-Language"
#define	HEADER_CONTENT_LENGTH "Content-Length"
#define	HEADER_CONTENT_TYPE "Content-Type"

//Status codes reason phrases
#define PHRASE_OK "OK"
#define PHRASE_Created "Created"
#define PHRASE_Accepted "Accepted"
#define PHRASE_No_Content "No Content"
#define PHRASE_Bad_Request "Bad Request"
#define PHRASE_Not_Found "Not Found"
#define PHRASE_Internal_Server_Error "Internal_Server_Error"
#define PHRASE_Not_Implemented "Not Implemented"
#define PHRASE_Bad_Gateway "Bad Gateway"
#define PHRASE_Service_Unavailable "Service_Unavailable"
#define PHRASE_HTTP_Version_Not_Supported "HTTP Version Not Supported"