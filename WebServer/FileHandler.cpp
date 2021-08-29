#include <iostream>
#include "Methods.h"
#include <fstream>
#include <ostream>
#include <istream>
#include "StatusCodes.h"
using namespace std;

bool isFileExists(string path) {
	
	fstream file;
	bool isExist = flase;

	file.open(path, ios_base::_Nocreate);
	if (file) {
		isExist = true;
	}

	file.close();

	return isExist;
}	

int createAndWriteIntoAFile(string path, string content) {

	fstream file;
	int i;
	int response = HTTP_Not_Found;
	int isWriteSuccessful = -1;	
	bool isExits = isFileExists(path);

	file.open(path, ios_base::out);
	if (file) {
		isWriteSuccessful = writeIntoAFile(file, content);
		if (isWriteSuccessful > 0) {
			response = HTTP_OK;
		}
		else if (isWriteSuccessful == 0) {
			response == HTTP_Created;
		}
		else
		{
			response = HTTP_Not_Implemented;
		}
	}
	else {
		response = HTTP_Internal_Server_Error;
	}
	file.close();

	return response;
}

int deleteFile(string path) {

	fstream file;
	int i;
	bool isExits = isFileExists(path);
	int response = HTTP_Not_Found;

	if (isExits) {
		if (remove(path) == 0) {
			response = HTTP_No_Content;
		}
		else {
			response = HTTP_Accepted;
		}
	}
	
	return response;
}

int writeIntoAFile(fstream file, string content) {

	int isSuccessful = -1;
	int len = strlen(content);

	if (file.is_open()) {
		for (i = 0; i < len; i++)
		{
			file << content[i];
		}
		isSuccessful = len;
	}

	return isSuccessful;
}