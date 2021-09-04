#include <iostream>
#include <fstream>
#include "StatusCodes.h"
#include "HTTPFileHandler.h"
#include <filesystem>
#include <algorithm>



using namespace std;

string HTTPFileHandler::getFileInStream(int* statusCode, const Request& request)
{
	fstream file;
	string finalPath = getFinalPath(request);

	if(isFileExists(finalPath))
	{
		file.open(finalPath, ios_base::in);

		// convert file to string
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
		isSuccessful = static_cast<int>(content.size());
	}

	return isSuccessful;
}

bool HTTPFileHandler::isFileExists(const string& path) {
	
	fstream file;
	bool isExist = false;

	file.open(path, ios_base::_Nocreate);

	if (file.good()) {
		isExist = true;
	}

	file.close();

	return isExist;
}	

int HTTPFileHandler::createAndWriteIntoAFileForPUT(const Request& request, const string& content) {

	fstream file;
	int response = HTTP_Not_Found;
	int isWriteSuccessful = -1;
	string finalPath = getFinalPath(request);

	if (isFileExists(finalPath))
	{
		file.open(finalPath, ios_base::out);
		if (file.good()) {
			isWriteSuccessful = writeIntoAFile(file, content);
			if (isWriteSuccessful > 0) {
				response = HTTP_OK;
			}
			else if (isWriteSuccessful == 0)
			{
				response = HTTP_No_Content;
			}
			else
			{
				response = HTTP_Not_Implemented;
			}
		}
		else
		{
			response = HTTP_Internal_Server_Error;
		}
	}
	else 
	{
		//check folder and create whats needed
		file.open(finalPath, ios_base::out);
		if (file.good()) {
			isWriteSuccessful = writeIntoAFile(file, content);
			if (isWriteSuccessful >= 0) {
				response = HTTP_Created;
			}
			else
			{
				response = HTTP_Not_Implemented;
			}
		}
		else
		{
			response = HTTP_Internal_Server_Error;
		}
	}

	file.close();

	return response;
}

string HTTPFileHandler::getFinalPath(const Request& request)
{
	string finalPath(request.getPath());

	if (finalPath == "www/")
	{
		finalPath += "index.html";
	}

	if (request.getQueryParam("lang") == "en")
	{
		finalPath.replace(static_cast<int>(finalPath.size()) - 4, 4, "en.html");
	}
	else if (request.getQueryParam("lang") == "he")
	{
		finalPath.replace(static_cast<int>(finalPath.size()) - 4, 4, "he.html");
	}
	else if (request.getQueryParam("lang") == "fr")
	{
		finalPath.replace(static_cast<int>(finalPath.size()) - 4, 4, "fr.html");
	}
	return finalPath;
}

int HTTPFileHandler::deleteFile(const Request& request) {

	//add supporting removing folders
	fstream file;
	int response = HTTP_Not_Found;
	string finalPath = getFinalPath(request);

	if (isFileExists(finalPath)) {
		if (remove(finalPath.c_str()) == 0) {
			response = HTTP_No_Content;
		}
		else {
			response = HTTP_Accepted;
		}
	}
	
	return response;
}

