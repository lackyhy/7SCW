#ifndef INC_7SCW_FILE_MANAGER_H
#define INC_7SCW_FILE_MANAGER_H

#include "string"
#include "windows.h"

using namespace std;

struct CleanupResult {
    int deletedCount = 0;
    ULARGE_INTEGER cleanedSize = {0};
};

void file_manger();
void startSearch(const string& startPath);
// Non-interactive search entry: provide pattern directly (e.g., "SS::main.cpp")
void startSearchWithPattern(const string& pattern, const string& startPath = "");

ULARGE_INTEGER calculateFolderSize(const string& path);
CleanupResult cleanTempDirectory(const string& path);

#endif //INC_7SCW_FILE_MANAGER_H
