// 200 Codes
#define HTTP_OK 200
#define HTTP_Created 201
#define HTTP_Accepted 202
#define HTTP_Non_Authoritative_Information 203
#define HTTP_No_Content 204
#define HTTP_Reset_Content 205
#define HTTP_Partial_Content 206
#define HTTP_Multi_Status 207
#define HTTP_Already_Reported 208
#define HTTP_IM_Used 226

// 300 Codes
#define HTTP_Multiple_Choices 300
#define HTTP_Moved_Permanently 301
#define HTTP_Found 302
#define HTTP_See_Other 303
#define HTTP_Not_Modified 304
#define HTTP_Use_Proxy 305
#define HTTP_Temporary_Redirect 307
#define HTTP_Permanent_Redirect 308

// 400 Codes
#define HTTP_Bad_Request 400
#define HTTP_Unauthorized 401
#define HTTP_Payment_Required 402
#define HTTP_Forbidden 403
#define HTTP_Not_Found 404
#define HTTP_Method_Not_Allowed 405
#define HTTP_Not_Acceptable 406
#define HTTP_Proxy_Authentication_Required 407
#define HTTP_Request_Timeout 408
#define HTTP_Conflict 409
#define HTTP_Gone 410
#define HTTP_Length_Required 411
#define HTTP_Precondition_Failed 412
#define HTTP_Request_Entity_Too_Large 413
#define HTTP_Request_URI_Too_Long 414
#define HTTP_Unsupported_Media_Type 415
#define HTTP_Requested_Range_Not_Satisfiable 416
#define HTTP_Expectation_Failed 417
#define HTTP_Im_A_Teapot 418
#define HTTP_Misdirected_Request 421
#define HTTP_Unprocessable_Entity 422
#define HTTP_Locked 423
#define HTTP_Failed_Dependency 424
#define HTTP_Too_Early 425
#define HTTP_Upgrade_Required 426
#define HTTP_Precondition_Required 428
#define HTTP_Too_Many_Requests 429
#define HTTP_Request_Header_Fields_Too_Large 431
#define HTTP_Unavailable_For_Legal_Reasons 451

// 500 Codes
#define HTTP_Internal_Server_Error 500
#define HTTP_Not_Implemented 501
#define HTTP_Bad_Gateway 502
#define HTTP_Service_Unavailable 503
#define HTTP_Gateway_Timeout 504
#define HTTP_HTTP_Version_Not_Supported 505
#define HTTP_Variant_Also_Negotiates 506
#define HTTP_Insufficient_Storage 507
#define HTTP_Loop_Detected 508
#define HTTP_Not_Extended 510
#define HTTP_Network_Authentication_Required 511

enum StatusType
{
	SUCCESS = 200,
	REDIRECT = 300,
	CLIENT_ERROR = 400,
	SERVER_ERROR = 500
};