#pragma once

#include <iostream>
#include <fstream>
#include "StatusCodes.h"

using namespace std;

class HTTPFileHandler {
private:
	const string ROOT_PATH = "www";

public:
	string getFileInStream(const string& path, int* statusCode);
	int writeIntoAFile(fstream& file, const string& content);
	bool isFileExists(const string& path);
	int createAndWriteIntoAFileForPUT(const string& path, const string& content);
	int createAndWriteIntoAFileForPOST(const string& path, const string& content);
	int deleteFile(const string& path);
};
