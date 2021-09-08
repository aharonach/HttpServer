#include <iostream>
#include <fstream>
#include "StatusCodes.h"
#include "HTTPFileHandler.h"
#include <filesystem>
#include <algorithm>
#include <direct.h>

using namespace std;

string HTTPFileHandler::getFileInStream(int* statusCode, const Request& request)
{
	fstream file;
	string finalPath(getFinalPath(request));

	if (isFileExists(finalPath))
	{
		file.open(finalPath, ios_base::in);

		// put the file content into a string
		string stringToReturn((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());

		*statusCode = stringToReturn.size() > 0 ? HTTP_OK : HTTP_No_Content;
		return stringToReturn;
	}
	else
	{
		*statusCode = HTTP_Not_Found;
	}

	return "";
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
	string finalPath(getFinalPath(request));

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
			else {
				response = HTTP_Not_Implemented;
			}
		}
		else {
			response = HTTP_Internal_Server_Error;
		}
	}
	else
	{
		// Create directories if needed
		createDirectories(finalPath);

		//check folder and create whats needed
		file.open(finalPath, ios_base::out);
		if (file.good()) {
			isWriteSuccessful = writeIntoAFile(file, content);
			response = isWriteSuccessful >= 0 ? HTTP_Created : HTTP_Not_Implemented;
		}
		else if (finalPath.find(".html") == string::npos || finalPath.find(".txt") == string::npos) {
			response = HTTP_Created;
		}
		else {
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

	string langSupport(request.getQueryParam("lang"));

	if (langSupport == "en" || langSupport == "he" || langSupport == "fr")
	{
		finalPath.replace(static_cast<int>(finalPath.size()) - 4, 4, langSupport + ".html");
	}

	return finalPath;
}

int HTTPFileHandler::deleteFile(const Request& request)
{
	int response = HTTP_Not_Found;
	string finalPath = getFinalPath(request);

	if (isFileExists(finalPath)) {
		response = remove(finalPath.c_str()) == 0 ? HTTP_No_Content : HTTP_Accepted;
	}

	return response;
}

void HTTPFileHandler::createDirectories(const string& finalPath)
{
	stringstream pathStream(finalPath);
	stringstream directoryStream;

	for (string folderPath; getline(pathStream, folderPath, '/'); )
	{
		directoryStream << folderPath;

		if (folderPath != "www") {
			if (folderPath.find(".html") != string::npos || folderPath.find(".txt") != string::npos) {
				break;
			}

			string directoryPath(directoryStream.str());
			fstream directory(directoryPath);

			if (!directory.good()) {
				_mkdir(directoryPath.c_str());
			}

			directory.close();
		}

		directoryStream << "\\";
	}
}
