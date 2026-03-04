#include <iostream>
#include <string>
#include <vector>
#include <shlobj.h>
#include <conio.h>

#include <windows.h>
#include <conio.h>
#include <shlobj.h>
#include "../../h_file/file_manager/file_manager.h"
#include "../../Logger.h"

using namespace std;

string formatFileSize(DWORD size)
{
    const char *units[] = {"B", "KB", "MB", "GB", "TB"};
    int unit = 0;
    double fileSize = (double)size;

    while (fileSize >= 1024 && unit < 4)
    {
        fileSize /= 1024;
        unit++;
    }
    stringstream ss;
    ss << fixed << setprecision(2) << fileSize << " " << units[unit];
    return ss.str();
}

void clear_temp_file()
{
    Logger::info("Starting system cleanup");
    system("cls");

    cout << "Cleaning system temporary and junk files..." << endl
         << endl;
    CleanupResult totalResult = {0};
    char path[MAX_PATH];

    if (GetTempPathA(MAX_PATH, path) != 0)
    {
        cout << "[Processing] User Temp..." << endl;
        CleanupResult res = cleanTempDirectory(path);
        totalResult.deletedCount += res.deletedCount;
        totalResult.cleanedSize.QuadPart += res.cleanedSize.QuadPart;
    }

    if (GetWindowsDirectoryA(path, MAX_PATH) != 0)
    {
        string winTemp = string(path) + "\\Temp";
        cout << "[Processing] Windows System Temp..." << endl;
        CleanupResult res = cleanTempDirectory(winTemp);
        totalResult.deletedCount += res.deletedCount;
        totalResult.cleanedSize.QuadPart += res.cleanedSize.QuadPart;
    }

    if (GetWindowsDirectoryA(path, MAX_PATH) != 0)
    {
        string prefetch = string(path) + "\\Prefetch";
        cout << "[Processing] Windows Prefetch..." << endl;
        CleanupResult res = cleanTempDirectory(prefetch);
        totalResult.deletedCount += res.deletedCount;
        totalResult.cleanedSize.QuadPart += res.cleanedSize.QuadPart;
    }

    if (SHGetFolderPathA(NULL, CSIDL_RECENT, NULL, 0, path) == S_OK)
    {
        cout << "[Processing] Recent Items..." << endl;
        CleanupResult res = cleanTempDirectory(path);
        totalResult.deletedCount += res.deletedCount;
        totalResult.cleanedSize.QuadPart += res.cleanedSize.QuadPart;
    }

    cout << "[Processing] Emptying Recycle Bin..." << endl;
    SHEmptyRecycleBinA(NULL, NULL, SHERB_NOCONFIRMATION | SHERB_NOPROGRESSUI | SHERB_NOSOUND);

    cout << "\n========================================" << endl;
    cout << "Items deleted: " << totalResult.deletedCount << endl;
    cout << "Space reclaimed: " << formatFileSize(totalResult.cleanedSize.QuadPart) << endl;
    cout << "========================================" << endl;

    cout << "\nPress any key to return to menu...";
    _getch();
}