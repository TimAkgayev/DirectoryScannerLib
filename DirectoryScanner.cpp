#include<fstream>
#include<string>
#include<algorithm> //for std::replace
#include<windows.h>
#include<Shlwapi.h> // for PathIsDirectory()

#include "DirectoryScanner.h"

int ScanDirectory(std::string startingPath, int(*ProcessFileCallback)(std::string))
{

	//make sure there is a string
	if (startingPath.size() == 0) {
		return INVALID_PATH;
	}

	//if present replace all forward slashes with back slashes
	std::replace(startingPath.begin(), startingPath.end(), '/', '\\');

	//make sure there is no backslash at the end, pop_back removes last character of string
	if (startingPath[startingPath.size() - 1] == '\\')
		startingPath.pop_back();

	std::string directoryName;
	std::string directoryPath = startingPath;
	std::string wildCardPath = directoryPath + "\\*.*";//add wildcard, otherwise win7 fails

	//make sure there is a colon
	size_t colonPos = directoryPath.find(":", 0);

	//if no colon make sure that there is only one letter for drive
	if(colonPos == std::string::npos)
	{
		if (directoryPath.size() != 1)
			return INVALID_PATH;

		//directory name is just one letter
		directoryName = directoryPath;

		//add proper formatting to path
		directoryPath += ":\\";

	}
	else //if there is a column symbol then make sure that we don't just have something like "D:" or some nonsense like "D:file" or "noD:file"
	{
		//make sure colon is always in second place, which guarantees that drive only has one letter (in case of something like "noD:file")
		if (colonPos != 1)
			return INVALID_PATH;

		//if just one letter than colon, like "D:"
		if (directoryPath.size() == 2)
		{
			//add proper formating
			directoryPath += "\\";
		}
		else //make sure that the following character after colon is either a '\'  (in case of D:file)
		{
			if (!(directoryPath[2] == '\\'))
				return INVALID_PATH;
		}

		//seperate the name from full path of the root directory
		size_t lastSlash = directoryPath.find_last_of('\\', directoryPath.size() - 1);

		//if the slash is the last character in string that means we have a drive, just use first letter
		if (lastSlash == directoryPath.size() - 1)
			directoryName = directoryPath[0];
		else
			directoryName = directoryPath.substr(lastSlash + 1,std::string ::npos);
	}

	//now that everything is formatted properly, do a final check to make sure the string is actually an existing directory
	std::string testDir = directoryPath;

	//to test the dir must end with a slash, so since we removed it previous just temporarily add it back in
	if (directoryPath[directoryPath.size() - 1] != '\\')
		testDir += '\\';

	if (!PathIsDirectory(testDir.c_str()))
		return PATH_NOT_DIRECTORY;
	

	//Find first file
	/////////////////////////////////////////////////////////////////////////////////////////

	WIN32_FIND_DATA findData;
	HANDLE currentFileHandle;
	currentFileHandle = FindFirstFile(wildCardPath.c_str(), &findData);


	//directory always contains 2 hidden directories (. and ..), so if nothins is found there is something wrong
	if (currentFileHandle == INVALID_HANDLE_VALUE)
	{
		return 1;
	}

	//encountered a directory, descend recursively
	if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	{

		std::string s = findData.cFileName;
		//there are always a few hidden directories which we must avoid searching
		if (directoryName.compare(findData.cFileName) == 0 || s.compare(".") == 0 || s.compare("..") == 0)
		{
			//do nothing, let it fall through
		}
		else
			ScanDirectory(directoryPath + "\\" + findData.cFileName, ProcessFileCallback);

	}
	else if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
	{
		//process the file according accordingly
		ProcessFileCallback(directoryPath + "\\" + findData.cFileName);
		
	}

	//Find next file
	/////////////////////////////////////////////////////////////////////////////////////////

	while (FindNextFile(currentFileHandle, &findData))
	{
		if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) //encountered a directory, descend recursively
		{

			std::string st = findData.cFileName;
			if (directoryName.compare(findData.cFileName) == 0 || st.compare(".") == 0 || st.compare("..") == 0)
			{
				//do nothing, let it fall through
			}
			else
				ScanDirectory(directoryPath + "\\" + findData.cFileName, ProcessFileCallback);

		}
		else if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			
			ProcessFileCallback(directoryPath + "\\" + findData.cFileName);
			
		}
	}


	return 1;

}
