#pragma once

#include <iostream>
#include <fstream>
#include "StatusCodes.h"
#include "Request.h"
#include <algorithm>
#include <sys/types.h>
#include <sys/stat.h>



using namespace std;

class HTTPFileHandler {
private:
	const string ROOT_PATH = "www";
	void createDirectories(const string& path);

public:
	string getFileInStream(int* statusCode, const Request& request);
	int writeIntoAFile(fstream& file, const string& content);
	bool isFileExists(const string& path);
	int createAndWriteIntoAFileForPUT(const Request& request, const string& content);
	int deleteFile(const Request& request);
	string getFinalPath(const Request& request);
};
