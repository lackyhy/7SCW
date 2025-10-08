//
// Created by User on 29.09.2025.
//

#ifndef INC_7SCW_FILE_MANAGER_H
#define INC_7SCW_FILE_MANAGER_H

#include "string"

struct CleanupResult {
    int deletedCount = 0;
    ULARGE_INTEGER cleanedSize = {0};
};

void file_manger();

ULARGE_INTEGER calculateFolderSize(const std::string& path);
CleanupResult cleanTempDirectory(const std::string& path);

#endif //INC_7SCW_FILE_MANAGER_H
