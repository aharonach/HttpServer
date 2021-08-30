#include <iostream>
#include <fstream>
#include "StatusCodes.h"
#include "FileHandler.h"

using namespace std;

string HTTPFileHandler::getFileInStream(const string& path, int* statusCode)
{
	fstream file;
	if(isFileExists(path))
	{
		file.open(path, ios_base::in);
		string stringToReturn((std::istreambuf_iterator<char>(file)),
			(std::istreambuf_iterator<char>()));
		if (stringToReturn.size() == 0) 
		{
			stringToReturn = "No Content";
			*statusCode = HTTP_No_Content;
		}
		else
		{
			*statusCode = HTTP_OK;
		}
		return stringToReturn;
	}
	else
	{
		*statusCode = HTTP_Not_Found;
		return "Not Found";
	}


}

int HTTPFileHandler::writeIntoAFile(fstream& file, const string& content) {

	int isSuccessful = -1;

	if (file.is_open()) {
		file << content;
		isSuccessful = content.size();
	}

	return isSuccessful;
}

bool HTTPFileHandler::isFileExists(const string& path) {
	
	fstream file;
	bool isExist = false;

	file.open(path, ios_base::_Nocreate);
	if (file) {
		isExist = true;
	}

	file.close();

	return isExist;
}	

int HTTPFileHandler::createAndWriteIntoAFile(const string& path, const string& content) {

	fstream file;
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
			response = HTTP_Created;
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

int HTTPFileHandler::deleteFile(const string& path) {

	fstream file;
	bool isExists = isFileExists(path);
	int response = HTTP_Not_Found;

	if (isExists) {
		if (remove(path.c_str()) == 0) {
			response = HTTP_No_Content;
		}
		else {
			response = HTTP_Accepted;
		}
	}
	
	return response;
}