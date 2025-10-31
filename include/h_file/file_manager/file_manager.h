//
// Created by User on 29.09.2025.
//

#ifndef INC_7SCW_FILE_MANAGER_H
#define INC_7SCW_FILE_MANAGER_H

#include "string"
#include "windows.h"

struct CleanupResult {
    int deletedCount = 0;
    ULARGE_INTEGER cleanedSize = {0};
};


void file_manger();
void startSearch(const std::string& startPath);
// Non-interactive search entry: provide pattern directly (e.g., "SS::main.cpp")
void startSearchWithPattern(const std::string& pattern, const std::string& startPath = "");

ULARGE_INTEGER calculateFolderSize(const std::string& path);
CleanupResult cleanTempDirectory(const std::string& path);

#endif //INC_7SCW_FILE_MANAGER_H
