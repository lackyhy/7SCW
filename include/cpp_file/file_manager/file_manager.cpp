#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <iomanip>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <cmath>
#include <windows.h>
#include <wincrypt.h>
#include <conio.h>

#include "../../h_file/file_manager/file_manager.h"
#include "../../h_file/main.h"
#include "../../Logger.h"

using namespace std;

#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "shell32.lib")

#ifndef CALG_SHA_256
#define CALG_SHA_256 0x0000800c
#endif

//  structures:
struct FileInfo {
    string name;
    string fullPath;
    bool isDirectory;
    DWORD attributes;
    FILETIME lastWriteTime;
};

struct DiskInfo {
    string drive;
    string label;
    double freeSpace;
    double totalSpace;
};

// --- Global Utilities & Cleanup Helpers ---

ULARGE_INTEGER calculateFolderSize(const string& path) {
    Logger::info("Calculating folder size for: " + path);
    ULARGE_INTEGER totalSize = { 0 };
    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA((path + "\\*").c_str(), &findData);

    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (strcmp(findData.cFileName, ".") != 0 && strcmp(findData.cFileName, "..") != 0) {
                string fullPath = path + "\\" + findData.cFileName;

                if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    ULARGE_INTEGER subFolderSize = calculateFolderSize(fullPath);
                    totalSize.QuadPart += subFolderSize.QuadPart;
                }
                else {
                    ULARGE_INTEGER fileSize;
                    fileSize.LowPart = findData.nFileSizeLow;
                    fileSize.HighPart = findData.nFileSizeHigh;
                    totalSize.QuadPart += fileSize.QuadPart;
                }
            }
        } while (FindNextFileA(hFind, &findData));
        FindClose(hFind);
    }
    return totalSize;
}

CleanupResult cleanTempDirectory(const string& path) {
    Logger::info("Starting cleanup of directory: " + path);
    CleanupResult result;
    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA((path + "\\*").c_str(), &findData);

    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (strcmp(findData.cFileName, ".") != 0 && strcmp(findData.cFileName, "..") != 0) {
                string fullPath = path + "\\" + findData.cFileName;

                if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    CleanupResult subResult = cleanTempDirectory(fullPath);
                    result.deletedCount += subResult.deletedCount;
                    result.cleanedSize.QuadPart += subResult.cleanedSize.QuadPart;

                    if (RemoveDirectoryA(fullPath.c_str())) {
                        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                        cout << "Removed directory: " << fullPath << endl;
                        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
                    }
                }
                else {
                    ULARGE_INTEGER fileSize;
                    fileSize.LowPart = findData.nFileSizeLow;
                    fileSize.HighPart = findData.nFileSizeHigh;

                    if (DeleteFileA(fullPath.c_str())) {
                        result.deletedCount++;
                        result.cleanedSize.QuadPart += fileSize.QuadPart;
                        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                        cout << "Deleted file: " << fullPath << endl;
                        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
                    }
                }
            }
        } while (FindNextFileA(hFind, &findData));
        FindClose(hFind);
    }
    return result;
}

// --- Class FileManager ---

class FileManager {
private:
    vector<FileInfo> results;

    static void setColor(WORD color) {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hConsole, color);
    }

    static string getProgressBar(double percentage, int width = 15) {
        if (percentage < 0.0) percentage = 0.0;
        if (percentage > 100.0) percentage = 100.0;
        int filled = (int)round((percentage / 100.0) * width);
        string bar = "[";
        for (int i = 0; i < width; ++i) {
            bar += (i < filled) ? "=" : "-";
        }
        bar += "] ";
        ostringstream ss;
        ss << fixed << setprecision(1) << percentage << "%";
        return bar + ss.str();
    }

    DiskInfo getDiskInfo(const string& drive) {
        DiskInfo info;
        info.drive = drive;
        info.freeSpace = 0;
        info.totalSpace = 0;

        ULARGE_INTEGER freeBytesAvailable, totalBytes, totalFreeBytes;
        if (GetDiskFreeSpaceExA(drive.c_str(), &freeBytesAvailable, &totalBytes, &totalFreeBytes)) {
            info.freeSpace = (double)freeBytesAvailable.QuadPart / (1024.0 * 1024.0 * 1024.0);
            info.totalSpace = (double)totalBytes.QuadPart / (1024.0 * 1024.0 * 1024.0);
        }

        char volumeName[MAX_PATH + 1] = { 0 };
        DWORD volumeSerialNumber = 0;
        DWORD maximumComponentLength = 0;
        DWORD fileSystemFlags = 0;
        char fileSystemName[MAX_PATH + 1] = { 0 };

        if (GetVolumeInformationA(drive.c_str(), volumeName, sizeof(volumeName),
                                  &volumeSerialNumber, &maximumComponentLength, &fileSystemFlags,
                                  fileSystemName, sizeof(fileSystemName))) {
            info.label = volumeName;
        }

        return info;
    }

    vector<DiskInfo> getAvailableDrives() {
        vector<DiskInfo> drives;
        DWORD driveMask = GetLogicalDrives();

        for (char letter = 'A'; letter <= 'Z'; letter++) {
            if (driveMask & 1) {
                string drive = string(1, letter) + ":\\";
                if (GetDriveTypeA(drive.c_str()) != DRIVE_UNKNOWN) {
                    drives.push_back(getDiskInfo(drive));
                }
            }
            driveMask >>= 1;
        }

        return drives;
    }

    vector<FileInfo> getDirectoryContents(const string& path) {
        vector<FileInfo> files;
        string searchPath = path + "\\*";
        WIN32_FIND_DATAA findData;
        HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);

        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                if (strcmp(findData.cFileName, ".") != 0) {
                    FileInfo file;
                    file.name = findData.cFileName;
                    file.fullPath = path + "\\" + findData.cFileName;
                    file.isDirectory = (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
                    file.attributes = findData.dwFileAttributes;
                    file.lastWriteTime = findData.ftLastWriteTime;
                    files.push_back(file);
                }
            } while (FindNextFileA(hFind, &findData));
            FindClose(hFind);
        }

        // Sort: directories first, then files alphabetically
        sort(files.begin(), files.end(), [](const FileInfo& a, const FileInfo& b) {
            if (a.name == "..") return true;
            if (b.name == "..") return false;
            if (a.isDirectory != b.isDirectory)
                return a.isDirectory > b.isDirectory;
            return a.name < b.name;
        });

        return files;
    }

    vector<FileInfo> searchFiles(const string& startPath, const string& searchPattern, bool foldersOnly = false, bool filesOnly = false, bool exactMatch = false) {
        vector<FileInfo> results;
        string searchPath = startPath + "\\*";
        WIN32_FIND_DATAA findData;
        HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);

        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                if (strcmp(findData.cFileName, ".") != 0 && strcmp(findData.cFileName, "..") != 0) {
                    string fullPath = startPath + "\\" + findData.cFileName;
                    bool isDirectory = (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;

                    bool matches = false;
                    string nameStr = findData.cFileName;
                    string patStr = searchPattern;

                    if (exactMatch) {
                        matches = (nameStr == patStr);
                    } else {
                        string nameLower = nameStr;
                        string patLower = patStr;
                        transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
                        transform(patLower.begin(), patLower.end(), patLower.begin(), ::tolower);
                        matches = (nameLower.find(patLower) != string::npos);
                    }

                    if (matches) {
                        if ((foldersOnly && isDirectory) || (filesOnly && !isDirectory) || (!foldersOnly && !filesOnly)) {
                            FileInfo file;
                            file.name = findData.cFileName;
                            file.fullPath = fullPath;
                            file.isDirectory = isDirectory;
                            file.attributes = findData.dwFileAttributes;
                            file.lastWriteTime = findData.ftLastWriteTime;
                            results.push_back(file);
                        }
                    }

                    if (isDirectory) {
                        vector<FileInfo> subResults = searchFiles(fullPath, searchPattern, foldersOnly, filesOnly, exactMatch);
                        results.insert(results.end(), subResults.begin(), subResults.end());
                    }
                }
            } while (FindNextFileA(hFind, &findData));
            FindClose(hFind);
        }
        return results;
    }

    // --- Helper Utilities ---

    static WORD getFileColor(const FileInfo& file) {
        if (file.attributes & (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM)) {
            return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE; // Dim Gray
        }
        if (file.isDirectory) {
            return FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY; // Bright Cyan
        }
        
        string ext = "";
        size_t dotPos = file.name.find_last_of('.');
        if (dotPos != string::npos) {
            ext = file.name.substr(dotPos);
            transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        }
        
        if (ext == ".exe" || ext == ".bat" || ext == ".cmd" || ext == ".ps1" || ext == ".msi") {
            return FOREGROUND_GREEN | FOREGROUND_INTENSITY; // Bright Green
        }
        if (ext == ".zip" || ext == ".rar" || ext == ".7z" || ext == ".tar" || ext == ".gz" || ext == ".iso") {
            return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY; // Yellow
        }
        if (ext == ".txt" || ext == ".log" || ext == ".ini" || ext == ".json" || ext == ".cpp" || ext == ".h" || ext == ".xml") {
            return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY; // Bright White
        }
        
        return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
    }

    static string formatFileSize(ULARGE_INTEGER size) {
        return formatFileSize(size.QuadPart);
    }

    static string formatFileSize(ULONGLONG size) {
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

    static string formatFileTime(const FILETIME& ft) {
        SYSTEMTIME st;
        FileTimeToSystemTime(&ft, &st);

        tm timeInfo = {};
        timeInfo.tm_year = st.wYear - 1900;
        timeInfo.tm_mon = st.wMonth - 1;
        timeInfo.tm_mday = st.wDay;
        timeInfo.tm_hour = st.wHour;
        timeInfo.tm_min = st.wMinute;
        timeInfo.tm_sec = st.wSecond;

        char buffer[100];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeInfo);
        return string(buffer);
    }

    static string getFileType(const FileInfo& file) {
        if (file.isDirectory) return "Folder";
        size_t dotPos = file.name.find_last_of('.');
        if (dotPos != string::npos) {
            return file.name.substr(dotPos + 1) + " File";
        }
        return "File";
    }

    // --- Core Operations: Copy, Move, Delete, Rename, Hash, Attributes, Preview ---

    static bool recursiveCopyHelper(const string& srcPath, const string& destPath) {
        DWORD dwAttrs = GetFileAttributesA(srcPath.c_str());
        if (dwAttrs == INVALID_FILE_ATTRIBUTES) return false;

        if (dwAttrs & FILE_ATTRIBUTE_DIRECTORY) {
            CreateDirectoryA(destPath.c_str(), NULL);
            WIN32_FIND_DATAA findData;
            HANDLE hFind = FindFirstFileA((srcPath + "\\*").c_str(), &findData);
            if (hFind != INVALID_HANDLE_VALUE) {
                do {
                    if (strcmp(findData.cFileName, ".") != 0 && strcmp(findData.cFileName, "..") != 0) {
                        string subSrc = srcPath + "\\" + findData.cFileName;
                        string subDest = destPath + "\\" + findData.cFileName;
                        recursiveCopyHelper(subSrc, subDest);
                    }
                } while (FindNextFileA(hFind, &findData));
                FindClose(hFind);
            }
            return true;
        } else {
            return (CopyFileA(srcPath.c_str(), destPath.c_str(), FALSE) != 0);
        }
    }

    static void recursiveDeleteHelper(const string& path) {
        WIN32_FIND_DATAA findData;
        HANDLE hFind = FindFirstFileA((path + "\\*").c_str(), &findData);

        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                if (strcmp(findData.cFileName, ".") != 0 && strcmp(findData.cFileName, "..") != 0) {
                    string fullPath = path + "\\" + findData.cFileName;
                    if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                        recursiveDeleteHelper(fullPath);
                        RemoveDirectoryA(fullPath.c_str());
                    } else {
                        DeleteFileA(fullPath.c_str());
                    }
                }
            } while (FindNextFileA(hFind, &findData));
            FindClose(hFind);
        }
    }

    void copyItem(const string& srcPath) {
        system("cls");
        cout << "Copy Item: " << srcPath << endl;
        cout << "Enter Destination Folder Path: ";
        string destFolder;
        getline(cin, destFolder);

        if (destFolder.empty()) {
            cout << "Destination folder path cannot be empty!\n";
            cout << "Press any key to continue...";
            _getch();
            return;
        }

        size_t lastSlash = srcPath.find_last_of("\\");
        string itemName = (lastSlash != string::npos) ? srcPath.substr(lastSlash + 1) : srcPath;
        string targetPath = destFolder + "\\" + itemName;

        Logger::info("Copying " + srcPath + " to " + targetPath);
        if (recursiveCopyHelper(srcPath, targetPath)) {
            setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
            cout << "\n[+] Item copied successfully to: " << targetPath << endl;
        } else {
            setColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
            cout << "\n[-] Failed to copy item. Error: " << GetLastError() << endl;
        }
        setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        cout << "Press any key to continue...";
        _getch();
    }

    void moveItem(const string& srcPath) {
        system("cls");
        cout << "Move Item: " << srcPath << endl;
        cout << "Enter Destination Folder Path: ";
        string destFolder;
        getline(cin, destFolder);

        if (destFolder.empty()) {
            cout << "Destination folder path cannot be empty!\n";
            cout << "Press any key to continue...";
            _getch();
            return;
        }

        size_t lastSlash = srcPath.find_last_of("\\");
        string itemName = (lastSlash != string::npos) ? srcPath.substr(lastSlash + 1) : srcPath;
        string targetPath = destFolder + "\\" + itemName;

        Logger::info("Moving " + srcPath + " to " + targetPath);
        if (MoveFileExA(srcPath.c_str(), targetPath.c_str(), MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING)) {
            setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
            cout << "\n[+] Item moved successfully to: " << targetPath << endl;
        } else {
            setColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
            cout << "\n[-] Failed to move item. Error: " << GetLastError() << endl;
        }
        setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        cout << "Press any key to continue...";
        _getch();
    }

    void deleteFile(const string& path) {
        system("cls");
        cout << "Are you sure you want to delete: " << path << "? (Y/N): ";
        char choice = _getch();
        cout << endl;

        if (choice == 'Y' || choice == 'y') {
            DWORD attributes = GetFileAttributesA(path.c_str());
            if (attributes != INVALID_FILE_ATTRIBUTES) {
                if (attributes & FILE_ATTRIBUTE_DIRECTORY) {
                    recursiveDeleteHelper(path);
                    if (RemoveDirectoryA(path.c_str())) {
                        setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                        cout << "Directory deleted successfully!" << endl;
                    } else {
                        setColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
                        cout << "Failed to delete directory. Error: " << GetLastError() << endl;
                    }
                } else {
                    if (DeleteFileA(path.c_str())) {
                        setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                        cout << "File deleted successfully!" << endl;
                    } else {
                        setColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
                        cout << "Failed to delete file. Error: " << GetLastError() << endl;
                    }
                }
            } else {
                setColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
                cout << "Item not found: " << path << endl;
            }
            setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        }
        cout << "Press any key to continue...";
        _getch();
    }

    void renameFile(const string& oldPath) {
        system("cls");
        cout << "Rename: " << oldPath << endl;

        DWORD attributes = GetFileAttributesA(oldPath.c_str());
        if (attributes == INVALID_FILE_ATTRIBUTES) {
            cout << "Error: Item not found!" << endl;
            cout << "Press any key to continue...";
            _getch();
            return;
        }

        cout << "Enter new name: ";
        string newName;
        getline(cin, newName);

        if (newName.empty()) {
            cout << "Error: New name cannot be empty!" << endl;
            cout << "Press any key to continue...";
            _getch();
            return;
        }

        string newPath = oldPath.substr(0, oldPath.find_last_of("\\") + 1) + newName;

        if (GetFileAttributesA(newPath.c_str()) != INVALID_FILE_ATTRIBUTES) {
            cout << "Error: An item with this name already exists!" << endl;
            cout << "Press any key to continue...";
            _getch();
            return;
        }

        if (MoveFileA(oldPath.c_str(), newPath.c_str())) {
            setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
            cout << "Renamed successfully!" << endl;
        } else {
            setColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
            cout << "Failed to rename. Error: " << GetLastError() << endl;
        }
        setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

        cout << "Press any key to continue...";
        _getch();
    }

    static string calculateFileHash(const string& filePath, ALG_ID algId) {
        HANDLE hFile = CreateFileA(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
        if (hFile == INVALID_HANDLE_VALUE) return "Failed to open file";

        HCRYPTPROV hProv = 0;
        HCRYPTHASH hHash = 0;
        if (!CryptAcquireContextA(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
            CloseHandle(hFile);
            return "CryptAcquireContext failed";
        }

        if (!CryptCreateHash(hProv, algId, 0, 0, &hHash)) {
            CryptReleaseContext(hProv, 0);
            CloseHandle(hFile);
            return "CryptCreateHash failed";
        }

        BYTE buffer[8192];
        DWORD bytesRead = 0;
        while (ReadFile(hFile, buffer, sizeof(buffer), &bytesRead, NULL) && bytesRead > 0) {
            CryptHashData(hHash, buffer, bytesRead, 0);
        }

        BYTE hashVal[64];
        DWORD hashLen = sizeof(hashVal);
        string result = "";
        if (CryptGetHashParam(hHash, HP_HASHVAL, hashVal, &hashLen, 0)) {
            ostringstream ss;
            for (DWORD i = 0; i < hashLen; i++) {
                ss << hex << uppercase << setw(2) << setfill('0') << (int)hashVal[i];
            }
            result = ss.str();
        } else {
            result = "Failed to compute hash";
        }

        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        CloseHandle(hFile);
        return result;
    }

    void showFileHashes(const string& filePath) {
        system("cls");
        setColor(FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
        cout << "================================================================================\n";
        cout << "                          FILE CHECKSUM / HASH TOOL                             \n";
        cout << "================================================================================\n";
        setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
        cout << "File: " << filePath << "\n\n";

        cout << "Calculating MD5 checksum...\n";
        string md5 = calculateFileHash(filePath, CALG_MD5);
        setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        cout << "  MD5:    " << md5 << "\n\n";

        setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
        cout << "Calculating SHA-256 checksum...\n";
        string sha256 = calculateFileHash(filePath, CALG_SHA_256);
        setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        cout << "  SHA256: " << sha256 << "\n\n";

        setColor(FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
        cout << "================================================================================\n";
        setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        cout << "Press any key to continue...";
        _getch();
    }

    void previewTextFile(const string& filePath) {
        system("cls");
        setColor(FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
        cout << "================================================================================\n";
        cout << "  QUICK TEXT PREVIEW: " << filePath << "\n";
        cout << "================================================================================\n";
        setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

        ifstream inFile(filePath.c_str(), ios::binary);
        if (!inFile.is_open()) {
            cout << "Failed to open file for text reading.\n";
            cout << "Press any key to continue...";
            _getch();
            return;
        }

        string line;
        int lineCount = 0;
        while (getline(inFile, line) && lineCount < 200) {
            lineCount++;
            setColor(FOREGROUND_BLUE | FOREGROUND_INTENSITY);
            cout << right << setw(4) << lineCount << " | ";
            setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
            cout << line << "\n";
        }

        if (!inFile.eof() && lineCount >= 200) {
            setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
            cout << "\n[... Preview truncated at 200 lines ...]\n";
        }

        inFile.close();
        setColor(FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
        cout << "================================================================================\n";
        setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        cout << "Press any key to return...";
        _getch();
    }

    void manageFileAttributes(const string& path) {
        system("cls");
        DWORD attrs = GetFileAttributesA(path.c_str());
        if (attrs == INVALID_FILE_ATTRIBUTES) {
            cout << "Item not found or inaccessible.\n";
            cout << "Press any key to return...";
            _getch();
            return;
        }

        setColor(FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
        cout << "==================================================\n";
        cout << "        FILE ATTRIBUTES MANAGER: " << path << "\n";
        cout << "==================================================\n";
        setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
        cout << "Current Attributes:\n";
        cout << "  [ " << ((attrs & FILE_ATTRIBUTE_READONLY) ? "X" : " ") << " ] Read-Only\n";
        cout << "  [ " << ((attrs & FILE_ATTRIBUTE_HIDDEN) ? "X" : " ") << " ] Hidden\n";
        cout << "  [ " << ((attrs & FILE_ATTRIBUTE_SYSTEM) ? "X" : " ") << " ] System\n\n";

        cout << "Actions:\n";
        cout << "  1. Toggle Read-Only\n";
        cout << "  2. Toggle Hidden\n";
        cout << "  3. Toggle System\n";
        cout << "  4. Back\n\n";
        cout << "Choice (1-4): ";

        char choice = _getch();
        if (choice == '1') {
            if (attrs & FILE_ATTRIBUTE_READONLY) attrs &= ~FILE_ATTRIBUTE_READONLY;
            else attrs |= FILE_ATTRIBUTE_READONLY;
            SetFileAttributesA(path.c_str(), attrs);
            cout << "\n[+] Read-Only attribute updated!\n";
        } else if (choice == '2') {
            if (attrs & FILE_ATTRIBUTE_HIDDEN) attrs &= ~FILE_ATTRIBUTE_HIDDEN;
            else attrs |= FILE_ATTRIBUTE_HIDDEN;
            SetFileAttributesA(path.c_str(), attrs);
            cout << "\n[+] Hidden attribute updated!\n";
        } else if (choice == '3') {
            if (attrs & FILE_ATTRIBUTE_SYSTEM) attrs &= ~FILE_ATTRIBUTE_SYSTEM;
            else attrs |= FILE_ATTRIBUTE_SYSTEM;
            SetFileAttributesA(path.c_str(), attrs);
            cout << "\n[+] System attribute updated!\n";
        }
        cout << "Press any key to continue...";
        _getch();
    }

    void createNewFolder(const string& currentPath) {
        system("cls");
        cout << "Create new folder in: " << currentPath << endl;
        cout << "Enter folder name: ";
        string folderName;
        getline(cin, folderName);

        if (folderName.empty()) return;

        string newPath = currentPath + "\\" + folderName;
        if (CreateDirectoryA(newPath.c_str(), NULL)) {
            setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
            cout << "Folder created successfully!" << endl;
        } else {
            setColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
            cout << "Failed to create folder. Error: " << GetLastError() << endl;
        }
        setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        cout << "Press any key to continue...";
        _getch();
    }

    void createNewFile(const string& fullPath) {
        system("cls");
        cout << "Create new file in: " << fullPath << endl;
        cout << "Enter file name: ";
        string fileName;
        getline(cin, fileName);

        if (fileName.empty()) return;

        string targetPath = fullPath + "\\" + fileName;
        HANDLE hFile = CreateFileA(targetPath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            CloseHandle(hFile);
            setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
            cout << "File created successfully!" << endl;
        } else {
            setColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
            cout << "Failed to create file. Error: " << GetLastError() << endl;
        }
        setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        cout << "Press any key to continue...";
        _getch();
    }

    void showFileManagerHelp() {
        system("cls");
        setColor(FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
        cout << "================================================================================\n";
        cout << "                           FILE MANAGER HELP & GUIDELINES                       \n";
        cout << "================================================================================\n";
        setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
        cout << "  Navigation & Hotkeys:\n";
        cout << "    Up / Down Arrows  - Navigate files and directories\n";
        cout << "    Enter             - Open directory or execute file\n";
        cout << "    Left Arrow        - Open File Management Options Menu\n";
        cout << "    Right Arrow       - Show Detailed File/Folder Information\n";
        cout << "    P / p             - Quick Text File Preview (first 200 lines)\n";
        cout << "    O / o             - Additional Operations (Copy, Move, Delete, Rename)\n";
        cout << "    S / s             - Search Files across directories / all drives\n";
        cout << "    4                 - Refresh file and directory list\n";
        cout << "    b / B             - Go up one directory level\n";
        cout << "    h / H             - Show this help menu\n";
        cout << "    q / Q / Esc       - Exit / Return to previous menu\n\n";

        cout << "  File Color Indicators:\n";
        setColor(FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
        cout << "    [Folder]         - Bright Cyan\n";
        setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        cout << "    Executables      - Bright Green (.exe, .bat, .cmd, .ps1)\n";
        setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        cout << "    Archives         - Bright Yellow (.zip, .rar, .7z, .iso)\n";
        setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
        cout << "    Documents/Code   - Bright White (.txt, .log, .cpp, .h, .json)\n";
        setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        cout << "    System / Hidden  - Dim Gray\n\n";

        cout << "  Advanced File Tools:\n";
        cout << "    - MD5 & SHA-256 Checksum Calculator\n";
        cout << "    - File Attributes Manager (Read-Only, Hidden, System)\n";
        cout << "    - Copy Full Path to Clipboard\n";
        cout << "    - Open Location in Windows Explorer\n";
        setColor(FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
        cout << "================================================================================\n";
        setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        cout << "Press any key to return...";
        _getch();
    }

    void showSearchHelp() {
        system("cls");
        cout << "|------------------- Search Help -------------------|" << endl;
        cout << "|                                                   |" << endl;
        cout << "|  FOLDER::  Search for folders only                |" << endl;
        cout << "|  FILE::    Search for files only                  |" << endl;
        cout << "|  SS::      Exact search (case-sensitive)          |" << endl;
        cout << "|  No prefix Search for both files and folders      |" << endl;
        cout << "|                                                   |" << endl;
        cout << "|  Examples:                                        |" << endl;
        cout << "|  FOLDER::Folder                                  |" << endl;
        cout << "|  FILE::filename.txt                              |" << endl;
        cout << "|  SS::ExactFileName.exe                           |" << endl;
        cout << "|                                                   |" << endl;
        cout << "|---------------------------------------------------|" << endl;
        cout << "\n     Press Enter to continue...";
        _getch();
    }

    void startSearch(const string& startPath) {
        system("cls");
        cout << "Enter search pattern (type HELP:: for help): ";
        string searchPattern;
        getline(cin, searchPattern);

        if (searchPattern == "HELP::") {
            showSearchHelp();
            return;
        } else if (searchPattern == "q") { return; }

        bool foldersOnly = false;
        bool filesOnly = false;
        bool exactMatch = false;
        bool searchAllDrives = false;

        if (searchPattern.find("FOLDER::") == 0) {
            foldersOnly = true;
            searchPattern = searchPattern.substr(8);
        } else if (searchPattern.find("FILE::") == 0) {
            filesOnly = true;
            searchPattern = searchPattern.substr(6);
        } else if (searchPattern.find("SS::") == 0) {
            exactMatch = true;
            searchPattern = searchPattern.substr(4);
            searchAllDrives = true;
        }

        if (searchPattern.empty()) {
            cout << "Search pattern cannot be empty!" << endl;
            cout << "Press any key to continue...";
            _getch();
            return;
        }

        results.clear();
        if (searchAllDrives) {
            cout << "Searching across all drives... This may take a while." << endl;
            vector<DiskInfo> drives = getAvailableDrives();
            for (const auto& drive : drives) {
                try {
                    vector<FileInfo> driveResults = searchFiles(drive.drive, searchPattern, foldersOnly, filesOnly, exactMatch);
                    results.insert(results.end(), driveResults.begin(), driveResults.end());
                } catch (...) {
                    continue;
                }
            }
        } else {
            cout << "Searching in current directory... This may take a while." << endl;
            results = searchFiles(startPath, searchPattern, foldersOnly, filesOnly, exactMatch);
        }

        if (results.empty()) {
            cout << "No files found matching the pattern: " << searchPattern << endl;
            cout << "Press any key to continue...";
            _getch();
            return;
        }

        showSearchResults(results, searchPattern);
    }

    void showSearchResults(vector<FileInfo>& resultsList, const string& pattern) {
        int selectedIndex = 0;
        bool running = true;

        while (running) {
            system("cls");
            setColor(FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
            cout << "Search Results for '" << pattern << "' (Found " << resultsList.size() << " items):\n\n";
            setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

            for (size_t i = 0; i < resultsList.size() && i < 30; i++) {
                bool isSel = (i == (size_t)selectedIndex);
                if (isSel) setColor(BACKGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
                else setColor(getFileColor(resultsList[i]));

                cout << (isSel ? "> " : "  ") << setw(35) << left << resultsList[i].name << " " << resultsList[i].fullPath << endl;
                setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            }

            int key = _getch();
            if (key == 224) {
                key = _getch();
                switch (key) {
                    case 72: // Up
                        selectedIndex = (selectedIndex - 1 + resultsList.size()) % resultsList.size();
                        break;
                    case 80: // Down
                        selectedIndex = (selectedIndex + 1) % resultsList.size();
                        break;
                }
            } else if (key == 13) { // Enter
                if (selectedIndex < (int)resultsList.size()) {
                    if (resultsList[selectedIndex].isDirectory) {
                        browseDirectory(resultsList[selectedIndex].fullPath);
                    } else {
                        ShellExecuteA(NULL, "open", resultsList[selectedIndex].fullPath.c_str(), NULL, NULL, SW_SHOW);
                    }
                }
            } else if (key == 'q' || key == 'Q') {
                running = false;
            }
        }
    }

    void showAdditionalOperations(const string& currentPath) {
        vector<string> options = {
            "Create New Folder",
            "Create New File",
            "===================",
            "Copy Item",
            "Move Item",
            "Delete Item",
            "Rename Item",
            "===================",
            "Back"
        };
        int selectedIndex = 0;
        bool running = true;

        while (running) {
            system("cls");
            setColor(FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
            cout << "ADDITIONAL FILE OPERATIONS (" << currentPath << ")\n\n";
            setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

            for (size_t i = 0; i < options.size(); i++) {
                if (options[i] == "===================") {
                    setColor(FOREGROUND_BLUE | FOREGROUND_INTENSITY);
                    cout << " " << options[i] << endl;
                    setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
                    continue;
                }

                bool isSel = (i == (size_t)selectedIndex);
                if (isSel) setColor(BACKGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
                cout << (isSel ? "> " : "  ") << options[i] << endl;
                if (isSel) setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            }

            int key = _getch();
            if (key == 224) {
                key = _getch();
                switch (key) {
                    case 72:
                        do {
                            selectedIndex = (selectedIndex - 1 + options.size()) % options.size();
                        } while (options[selectedIndex] == "===================");
                        break;
                    case 80:
                        do {
                            selectedIndex = (selectedIndex + 1) % options.size();
                        } while (options[selectedIndex] == "===================");
                        break;
                }
            } else if (key == 13) {
                switch (selectedIndex) {
                    case 0: createNewFolder(currentPath); break;
                    case 1: createNewFile(currentPath); break;
                    case 3: { // Copy
                        cout << "\nEnter item name to copy: ";
                        string itemName;
                        getline(cin, itemName);
                        if (!itemName.empty()) copyItem(currentPath + "\\" + itemName);
                        break;
                    }
                    case 4: { // Move
                        cout << "\nEnter item name to move: ";
                        string itemName;
                        getline(cin, itemName);
                        if (!itemName.empty()) moveItem(currentPath + "\\" + itemName);
                        break;
                    }
                    case 5: { // Delete
                        cout << "\nEnter item name to delete: ";
                        string itemName;
                        getline(cin, itemName);
                        if (!itemName.empty()) deleteFile(currentPath + "\\" + itemName);
                        break;
                    }
                    case 6: { // Rename
                        cout << "\nEnter item name to rename: ";
                        string itemName;
                        getline(cin, itemName);
                        if (!itemName.empty()) renameFile(currentPath + "\\" + itemName);
                        break;
                    }
                    case 8: running = false; break;
                }
            } else if (key == 27 || key == 'q' || key == 'Q') {
                running = false;
            }
        }
    }

    void drawFileList(const vector<FileInfo>& files, const string& currentPath, int selectedIndex) {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
        int windowHeight = csbi.srWindow.Bottom - csbi.srWindow.Top - 5;
        if (windowHeight < 5) windowHeight = 15;

        int startIndex = max(0, selectedIndex - windowHeight / 2);
        if (startIndex + windowHeight >= (int)files.size()) {
            startIndex = max(0, (int)files.size() - windowHeight);
        }

        system("cls");
        setColor(FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
        cout << "Directory: " << currentPath << endl;
        setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        cout << "Keys: [Arrows] Navigate | [Enter] Open | [Left] Details | [Right] Info | [P] Preview | [O] Operations | [S] Search | [q] Back\n\n";

        for (int i = startIndex; i < min(startIndex + windowHeight, (int)files.size()); i++) {
            bool isSel = (i == selectedIndex);
            if (isSel) {
                setColor(BACKGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
            } else {
                setColor(getFileColor(files[i]));
            }

            cout << (isSel ? "> " : "  ");
            cout << setw(35) << left << (files[i].isDirectory ? "[" + files[i].name + "]" : files[i].name);
            cout << setw(45) << left << files[i].fullPath << endl;

            setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        }
    }

    void showFileInfo(const FileInfo& file) {
        system("cls");
        setColor(FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
        cout << "================================================================================\n";
        cout << "                         FILE / FOLDER INFORMATION                              \n";
        cout << "================================================================================\n";
        setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
        cout << "Name:       " << file.name << "\n";
        cout << "Full Path:  " << file.fullPath << "\n";
        cout << "Type:       " << getFileType(file) << "\n";

        if (!file.isDirectory) {
            WIN32_FIND_DATAA findData;
            HANDLE hFind = FindFirstFileA(file.fullPath.c_str(), &findData);
            if (hFind != INVALID_HANDLE_VALUE) {
                ULARGE_INTEGER fileSize;
                fileSize.LowPart = findData.nFileSizeLow;
                fileSize.HighPart = findData.nFileSizeHigh;

                cout << "Size:       " << formatFileSize(fileSize) << " (" << fileSize.QuadPart << " bytes)\n";
                cout << "Created:    " << formatFileTime(findData.ftCreationTime) << "\n";
                cout << "Modified:   " << formatFileTime(findData.ftLastWriteTime) << "\n";
                cout << "Accessed:   " << formatFileTime(findData.ftLastAccessTime) << "\n";

                cout << "\nAttributes: ";
                if (findData.dwFileAttributes & FILE_ATTRIBUTE_READONLY) cout << "[Read-Only] ";
                if (findData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) cout << "[Hidden] ";
                if (findData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) cout << "[System] ";
                if (findData.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) cout << "[Archive] ";
                cout << "\n";
                FindClose(hFind);
            }
        } else {
            int fileCount = 0, dirCount = 0;
            ULARGE_INTEGER totalSize = calculateFolderSize(file.fullPath);

            WIN32_FIND_DATAA findData;
            HANDLE hFind = FindFirstFileA((file.fullPath + "\\*").c_str(), &findData);
            if (hFind != INVALID_HANDLE_VALUE) {
                do {
                    if (strcmp(findData.cFileName, ".") != 0 && strcmp(findData.cFileName, "..") != 0) {
                        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) dirCount++;
                        else fileCount++;
                    }
                } while (FindNextFileA(hFind, &findData));
                FindClose(hFind);
            }

            cout << "Size:       " << formatFileSize(totalSize) << " (" << totalSize.QuadPart << " bytes)\n";
            cout << "Contains:   " << fileCount << " files and " << dirCount << " subdirectories\n";
        }

        setColor(FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
        cout << "================================================================================\n";
        setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        cout << "\nPress any key to return...";
        _getch();
    }

    void showFileManagement(const FileInfo& file) {
        vector<string> options = {
            "Open / Execute",
            "Copy Item",
            "Move Item",
            "Delete Item",
            "Rename Item",
            "Edit File Attributes",
            "Calculate Checksum (MD5 / SHA256)",
            "Preview Text Content",
            "Copy Path to Clipboard",
            "Open Location in Explorer",
            "Back"
        };
        int selectedIndex = 0;
        bool running = true;

        while (running) {
            system("cls");
            setColor(FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
            cout << "MANAGEMENT OPTIONS: " << file.name << "\n";
            cout << "Path: " << file.fullPath << "\n\n";
            setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

            for (size_t i = 0; i < options.size(); i++) {
                bool isSel = (i == (size_t)selectedIndex);
                if (isSel) setColor(BACKGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
                cout << (isSel ? "> " : "  ") << options[i] << endl;
                if (isSel) setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            }

            int key = _getch();
            if (key == 224) {
                key = _getch();
                switch (key) {
                    case 72: selectedIndex = (selectedIndex - 1 + options.size()) % options.size(); break;
                    case 80: selectedIndex = (selectedIndex + 1) % options.size(); break;
                }
            } else if (key == 13) {
                switch (selectedIndex) {
                    case 0: // Open
                        if (file.isDirectory) browseDirectory(file.fullPath);
                        else ShellExecuteA(NULL, "open", file.fullPath.c_str(), NULL, NULL, SW_SHOW);
                        break;
                    case 1: copyItem(file.fullPath); break;
                    case 2: moveItem(file.fullPath); break;
                    case 3: deleteFile(file.fullPath); running = false; break;
                    case 4: renameFile(file.fullPath); running = false; break;
                    case 5: manageFileAttributes(file.fullPath); break;
                    case 6: showFileHashes(file.fullPath); break;
                    case 7: previewTextFile(file.fullPath); break;
                    case 8: // Clipboard
                        if (OpenClipboard(NULL)) {
                            EmptyClipboard();
                            HGLOBAL hGlob = GlobalAlloc(GMEM_FIXED, file.fullPath.length() + 1);
                            if (hGlob) {
                                strcpy((char*)hGlob, file.fullPath.c_str());
                                SetClipboardData(CF_TEXT, hGlob);
                                GlobalFree(hGlob);
                                setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                                cout << "\n[+] Path copied to clipboard!\n";
                            }
                            CloseClipboard();
                        }
                        cout << "Press any key to continue...";
                        _getch();
                        break;
                    case 9: // Explorer
                        ShellExecuteA(NULL, "explore", file.isDirectory ? file.fullPath.c_str() : file.fullPath.substr(0, file.fullPath.find_last_of("\\")).c_str(), NULL, NULL, SW_SHOW);
                        break;
                    case 10: running = false; break;
                }
            } else if (key == 'p' || key == 'P') {
                previewTextFile(file.fullPath);
            } else if (key == 27 || key == 'q' || key == 'Q') {
                running = false;
            }
        }
    }

    void drawFileManager(const vector<DiskInfo>& drives, int selectedIndex) {
        system("cls");
        setColor(FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
        cout << "================================================================================\n";
        cout << "                            SYSTEM FILE MANAGER                                 \n";
        cout << "================================================================================\n";
        setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        cout << "Keys: [Up/Down] Choose Drive | [Enter] Open Drive | [Left/Right] Info | [S] Search | [q] Quit\n\n";

        for (size_t i = 0; i < drives.size(); i++) {
            bool isSel = (i == (size_t)selectedIndex);
            if (isSel) setColor(BACKGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);

            cout << (isSel ? "> " : "  ");
            cout << setw(4) << left << drives[i].drive;
            cout << setw(20) << left << (drives[i].label.empty() ? "Local Disk" : drives[i].label);

            double usedSpace = drives[i].totalSpace - drives[i].freeSpace;
            double usagePercentage = (drives[i].totalSpace > 0) ? (usedSpace / drives[i].totalSpace) * 100.0 : 0.0;

            if (!isSel) {
                if (usagePercentage >= 90.0) setColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
                else if (usagePercentage >= 70.0) setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                else setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
            }

            cout << getProgressBar(usagePercentage, 15) << " ";
            cout << fixed << setprecision(1) << usedSpace << " GB / " << drives[i].totalSpace << " GB\n";

            if (isSel) setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        }
        setColor(FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
        cout << "================================================================================\n";
        setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    }

    void browseDirectory(const string& path) {
        vector<FileInfo> files = getDirectoryContents(path);
        int selectedIndex = 0;
        bool running = true;
        string currentPath = path;
        bool needsUpdate = true;

        while (running) {
            if (needsUpdate) {
                drawFileList(files, currentPath, selectedIndex);
                needsUpdate = false;
            }

            int key = _getch();
            if (key == 224) {
                key = _getch();
                switch (key) {
                    case 72: // Up
                        if (!files.empty()) {
                            selectedIndex = (selectedIndex - 1 + files.size()) % files.size();
                            needsUpdate = true;
                        }
                        break;
                    case 80: // Down
                        if (!files.empty()) {
                            selectedIndex = (selectedIndex + 1) % files.size();
                            needsUpdate = true;
                        }
                        break;
                    case 75: // Left -> File Management
                        if (selectedIndex < (int)files.size()) {
                            showFileManagement(files[selectedIndex]);
                            files = getDirectoryContents(currentPath);
                            needsUpdate = true;
                        }
                        break;
                    case 77: // Right -> File Info
                        if (selectedIndex < (int)files.size()) {
                            showFileInfo(files[selectedIndex]);
                            needsUpdate = true;
                        }
                        break;
                }
            } else if (key == 13) { // Enter
                if (selectedIndex < (int)files.size()) {
                    if (files[selectedIndex].isDirectory) {
                        if (files[selectedIndex].name == "..") {
                            size_t lastSlash = currentPath.find_last_of("\\");
                            if (lastSlash != string::npos) {
                                currentPath = currentPath.substr(0, lastSlash);
                                if (currentPath.length() == 2 && currentPath[1] == ':') currentPath += "\\";
                            }
                        } else {
                            currentPath = files[selectedIndex].fullPath;
                        }
                        files = getDirectoryContents(currentPath);
                        selectedIndex = 0;
                        needsUpdate = true;
                    } else {
                        ShellExecuteA(NULL, "open", files[selectedIndex].fullPath.c_str(), NULL, NULL, SW_SHOW);
                    }
                }
            } else if (key == 'q' || key == 'Q') {
                running = false;
            } else if (key == 'b' || key == 'B') {
                size_t lastSlash = currentPath.find_last_of("\\");
                if (lastSlash != string::npos) {
                    currentPath = currentPath.substr(0, lastSlash);
                    if (currentPath.length() == 2 && currentPath[1] == ':') currentPath += "\\";
                    files = getDirectoryContents(currentPath);
                    selectedIndex = 0;
                    needsUpdate = true;
                }
            } else if (key == 's' || key == 'S') {
                startSearch(currentPath);
                files = getDirectoryContents(currentPath);
                needsUpdate = true;
            } else if (key == 'h' || key == 'H') {
                showFileManagerHelp();
                needsUpdate = true;
            } else if (key == '4') {
                files = getDirectoryContents(currentPath);
                needsUpdate = true;
            } else if (key == 'o' || key == 'O') {
                showAdditionalOperations(currentPath);
                files = getDirectoryContents(currentPath);
                needsUpdate = true;
            } else if (key == 'p' || key == 'P') {
                if (selectedIndex < (int)files.size() && !files[selectedIndex].isDirectory) {
                    previewTextFile(files[selectedIndex].fullPath);
                    needsUpdate = true;
                }
            }
        }
    }

public:
    void fileManager() {
        vector<DiskInfo> drives = getAvailableDrives();
        int selectedIndex = 0;
        bool running = true;
        bool needsUpdate = true;

        while (running) {
            if (needsUpdate) {
                drawFileManager(drives, selectedIndex);
                needsUpdate = false;
            }

            int key = _getch();
            if (key == 224) {
                key = _getch();
                switch (key) {
                    case 72: // Up
                        if (!drives.empty()) {
                            selectedIndex = (selectedIndex - 1 + drives.size()) % drives.size();
                            needsUpdate = true;
                        }
                        break;
                    case 80: // Down
                        if (!drives.empty()) {
                            selectedIndex = (selectedIndex + 1) % drives.size();
                            needsUpdate = true;
                        }
                        break;
                }
            } else if (key == 13) {
                if (selectedIndex < (int)drives.size()) {
                    browseDirectory(drives[selectedIndex].drive);
                    needsUpdate = true;
                }
            } else if (key == 'q' || key == 'Q') {
                running = false;
            } else if (key == '4') {
                drives = getAvailableDrives();
                needsUpdate = true;
            } else if (key == 's' || key == 'S') {
                startSearch("");
                needsUpdate = true;
            }
        }
    }

    void startSearchPublic(const string& startPath) { startSearch(startPath); }

    void startSearchNonInteractive(const string& pattern, const string& startPath) {
        string searchPattern = pattern;
        bool foldersOnly = false;
        bool filesOnly = false;
        bool exactMatch = false;
        bool searchAllDrives = true;

        if (searchPattern.find("FOLDER::") == 0) {
            foldersOnly = true;
            searchPattern = searchPattern.substr(8);
        } else if (searchPattern.find("FILE::") == 0) {
            filesOnly = true;
            searchPattern = searchPattern.substr(6);
        } else if (searchPattern.find("SS::") == 0) {
            exactMatch = true;
            searchPattern = searchPattern.substr(4);
        }

        if (searchPattern.empty()) return;

        system("cls");
        results.clear();
        if (searchAllDrives) {
            vector<DiskInfo> drives = getAvailableDrives();
            for (const auto& drive : drives) {
                try {
                    vector<FileInfo> driveResults = searchFiles(drive.drive, searchPattern, foldersOnly, filesOnly, exactMatch);
                    results.insert(results.end(), driveResults.begin(), driveResults.end());
                } catch (...) {}
            }
        } else {
            string base = startPath;
            if (base.empty()) {
                char currentPath[MAX_PATH];
                GetCurrentDirectoryA(MAX_PATH, currentPath);
                base = currentPath;
            }
            results = searchFiles(base, searchPattern, foldersOnly, filesOnly, exactMatch);
        }

        showSearchResults(results, searchPattern);
    }
};

FileManager FileManager;

void file_manger() {
    Logger::info("Starting file manager");
    FileManager.fileManager();
}

void startSearch(const string& startPath) {
    Logger::info("Starting file search from path: " + startPath);
    FileManager.startSearchPublic(startPath);
    Logger::info("File search completed");
}

void startSearchWithPattern(const string& pattern, const string& startPath) {
    Logger::info("Starting file search with pattern: " + pattern + " from path: " + startPath);
    FileManager.startSearchNonInteractive(pattern, startPath);
    Logger::info("Pattern file search completed");
}
