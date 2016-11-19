#pragma once
enum SCAN_RESULT {SCAN_OK = 0, INVALID_PATH, PATH_NOT_DIRECTORY};

int ScanDirectory(std::string startingPath, int(*ProcessFileCallback)(std::string));