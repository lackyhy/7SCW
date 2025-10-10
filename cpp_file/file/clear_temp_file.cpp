#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <ctime>

#include <windows.h>
#include <conio.h>
#include <shlobj.h>
#include "../../h_file/file_manager/file_manager.h"

using namespace std;


string formatFileSize(DWORD size) {
    const char* units[] = { "B", "KB", "MB", "GB", "TB" };
    int unit = 0;
    double fileSize = (double)size;

    while (fileSize >= 1024 && unit < 4) {
        fileSize /= 1024;
        unit++;
    }

    stringstream ss;
    ss << fixed << setprecision(2) << fileSize << " " << units[unit];
    return ss.str();
}


void clear_temp_file() {
    {
        system("cls");
        cout << "Cleaning temporary files..." << endl << endl;

        CleanupResult totalCleanupResult = {0}; // Initialize total result

        // Get system temp path
        char tempPath[MAX_PATH];
        if (GetTempPathA(MAX_PATH, tempPath) != 0) {
            cout << "System TEMP directory: " << tempPath << endl;
            CleanupResult systemResult = cleanTempDirectory(tempPath);
            totalCleanupResult.deletedCount += systemResult.deletedCount;
            totalCleanupResult.cleanedSize.QuadPart += systemResult.cleanedSize.QuadPart;
        }

        // Get Windows temp path
        char windowsTempPath[MAX_PATH];
        if (GetWindowsDirectoryA(windowsTempPath, MAX_PATH) != 0) {
            string winTemp = string(windowsTempPath) + "\\Temp";
            cout << "\nWindows TEMP directory: " << winTemp << endl;
            CleanupResult windowsResult = cleanTempDirectory(winTemp);
            totalCleanupResult.deletedCount += windowsResult.deletedCount;
            totalCleanupResult.cleanedSize.QuadPart += windowsResult.cleanedSize.QuadPart;
        }

        // Get user temp path
        char userTempPath[MAX_PATH];
        if (SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, userTempPath) == S_OK) {
            string userTemp = string(userTempPath) + "\\Temp";
            cout << "\nUser TEMP directory: " << userTemp << endl;
            CleanupResult userResult = cleanTempDirectory(userTemp);
            totalCleanupResult.deletedCount += userResult.deletedCount;
            totalCleanupResult.cleanedSize.QuadPart += userResult.cleanedSize.QuadPart;
        }

        cout << "\nTemporary files cleanup completed!" << endl;
        cout << "Summary:" << endl;
        cout << "  Deleted items: " << totalCleanupResult.deletedCount << endl;
        cout << "  Cleaned space: " << formatFileSize(totalCleanupResult.cleanedSize.QuadPart) << endl;

        cout << "Press any key to continue...";
        _getch();
        return;
    }
}