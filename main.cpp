#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include <windows.h>
#include <conio.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <tlhelp32.h>
#include <cstring>

#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <sstream>
#include <locale>
#include <algorithm>
#include <regex>
#include <chrono>
#include <thread>
#include <map>
#include <fstream>
#include <ctime>
#include "terminal_commands.h"
#include "SHOW_ALL_STARTUP.h"

using namespace std;

// Forward declarations


// Global flag to indicate Ctrl+C pressed
volatile BOOL g_ctrlCPressed = FALSE;

// Console control handler function
BOOL WINAPI CtrlHandler(DWORD fdwCtrlType) {
    switch (fdwCtrlType) {
        case CTRL_C_EVENT:
            g_ctrlCPressed = TRUE; // Set the flag
            return TRUE;           // Indicate that the signal was handled
        default:
            return FALSE;          // Pass on other signals
    }
}

// Forward declarations
struct FileInfo {
    string name;
    string fullPath;
    bool isDirectory;
    DWORD attributes;
    FILETIME lastWriteTime;
};

// Структура для хранения информации о процессе


// Function prototypes
void browseDirectory(const string& path);
void showFileInfo(const FileInfo& file);
void showFileManagement(const FileInfo& file);
void customTerminal();
void showStartupLocationsMenu();
void checkStartupFolder();
void checkTaskScheduler();
void checkRegistryStartup();
void checkShellUserinit();
void restoreStartupSettings();
void SHOW_ALL_STARTUP();
void SHOW_ALL_STARTUP_1();
void SHOW_ALL_STARTUP_2();

// Logging functions
void writeLog(const string& message);
string getCurrentTimestamp();
void createBackup();
void restoreFromBackup();

void gotoxy(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

// Function to hide cursor
void hideCursor() {
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
    cursorInfo.bVisible = false;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
}

// Function to show cursor
void showCursor() {
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
    cursorInfo.bVisible = true;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
}

void showHelp() {
    system("cls");
    cout << "   File Manager and System Tools Help" << endl << endl;
    cout << "   Main Menu:" << endl;
    cout << "      - Use Up/Down arrows to navigate" << endl;
    cout << "      - Enter to select an option" << endl;
    cout << "      - Right arrow to open custom terminal" << endl << endl;

    cout << "   File Manager:" << endl;
    cout << "      - Navigate directories using Up/Down arrows" << endl;
    cout << "      - Enter to open folders or select files" << endl;
    cout << "      - 'q' to return to previous menu" << endl;
    cout << "      - 'b' to go back in directory history" << endl;
    cout << "      - Right arrow on a file/folder for detailed info" << endl;
    cout << "      - Left arrow on a disk for disk management" << endl;
    cout << "      - 'OTHER' option for additional operations" << endl;
    cout << "      - 'S' to search for files on the computer" << endl << endl;

    cout << "   System Tools:" << endl;
    cout << "       - Check Startup Locations: Interactive examination of startup locations with management commands" << endl;
    cout << "         * Startup Folders: View and open startup folders" << endl;
    cout << "         * Registry: View, add, and delete registry startup entries" << endl;
    cout << "         * Task Scheduler: View and manage scheduled startup tasks" << endl;
    cout << "         * Shell/Userinit: Check and restore critical system values" << endl;
    cout << "       - Clear TEMP Files: Remove temporary files to free up disk space" << endl;
    cout << "       - System Info: Display system information" << endl;
    cout << "       - Users: List system users" << endl << endl;

    cout << "   Key Bindings:" << endl;
    cout << "       - Up Arrow: Move selection up" << endl;
    cout << "       - Down Arrow: Move selection down" << endl;
    cout << "       - Enter: Select/Confirm" << endl;
    cout << "       - Right Arrow: Show detailed info (in File Manager)" << endl;
    cout << "       - Left Arrow: Disk management (in File Manager)" << endl;
    cout << "       - '4': Refresh list of files/drives" << endl;
    cout << "       - 'q': Quit/Return to previous menu" << endl;
    cout << "       - 'b': Go back (in File Manager)" << endl;
    cout << "       - 'h': Show this help menu" << endl << endl;

    cout << "   Custom Terminal:" << endl;
    cout << "       - Available from main menu by pressing right arrow" << endl;
    cout << "       - Type 'exit' or 'quit' to return to main menu" << endl;
    cout << "       - Supports bash-like commands and Windows commands" << endl;
    cout << "       - Type 'help' in terminal for available commands" << endl << endl;

    cout << "   Additional Programs:" << endl;
    cout << "       - Access to various utilities such as SimpleUnlocker, Registry Workshop, ProcessHacker, etc." << endl << endl;

    cout << "       Press any key to return to the main menu..." << endl;
    _getch();
}

// Function to draw menu
void drawMenu(const vector<string>& menuItems, int selectedIndex) {
    system("cls");
    cout << "Main Menu" << endl;
    cout << "Use Up and Down arrows to navigate, Enter to select, 'h' for help, 'q' to exit" << endl << endl;

    for (int i = 0; i < menuItems.size(); i++) {
        if (i == selectedIndex) {
            cout << ">";
        }
        else {
            cout << "   ";
        }
        cout << menuItems[i] << endl;
    }

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    int consoleWidth = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    cout << "\n\n";
    cout << endl;


    for(int i = 0; i < consoleWidth; ++i) {
        cout << "-";
    }

    cout << "Version: 8.3.0 | Created by: LCKY |GitHub: https://github.com/lackyhy/7SCW" << endl;

    for(int i = 0; i < consoleWidth; ++i) {
        cout << "-";
    }
}

struct DiskInfo {
    string drive;
    string label;
    double freeSpace;
    double totalSpace;
};

DiskInfo getDiskInfo(const string& drive) {
    DiskInfo info;
    info.drive = drive;

    ULARGE_INTEGER freeBytesAvailable, totalBytes, totalFreeBytes;
    if (GetDiskFreeSpaceExA(drive.c_str(), &freeBytesAvailable, &totalBytes, &totalFreeBytes)) {
        info.freeSpace = (double)freeBytesAvailable.QuadPart / (1024 * 1024 * 1024);
        info.totalSpace = (double)totalBytes.QuadPart / (1024 * 1024 * 1024);
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

void drawFileManager(const vector<DiskInfo>& drives, int selectedIndex) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    int windowHeight = csbi.srWindow.Bottom - csbi.srWindow.Top - 4; // -4 for header and footer
    int startIndex = max(0, selectedIndex - windowHeight / 2);
    if (startIndex + windowHeight >= drives.size()) {
        startIndex = max(0, (int)drives.size() - windowHeight);
    }

    system("cls");
    setlocale(LC_ALL, "RU");
    cout << "File Manager" << endl;
    cout << "Use Up/Down arrows to navigate, Left/Right arrows for info, Enter to select, 'q' to quit, 'h' for help." << endl << endl;

    for (int i = startIndex; i < min(startIndex + windowHeight, (int)drives.size()); i++) {
        if (i == selectedIndex) {
            cout << ">";
        }
        else {
            cout << "   ";
        }

        cout << setw(3) << drives[i].drive << "  ";
        cout << setw(20) << left << drives[i].label << "  ";

        // Calculate used space and usage percentage
        double usedSpace = drives[i].totalSpace - drives[i].freeSpace;
        double usagePercentage = (usedSpace / drives[i].totalSpace) * 100;

        // Set color based on usage percentage
        if (usagePercentage >= 90) {
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_INTENSITY);
        }
        else if (usagePercentage >= 70) {
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        }
        else if (usagePercentage >= 50) {
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN);
        }
        else {
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        }

        cout << fixed << setprecision(2) << setw(8) << usedSpace << "GB/"
             << setw(8) << drives[i].totalSpace << " GB ";

        // Reset color to default
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

        cout << "Info >" << endl;
    }
}

vector<FileInfo> getDirectoryContents(const string& path) {
    vector<FileInfo> files;
    string searchPath = path + "\\*";
    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);

    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (strcmp(findData.cFileName, ".") != 0 && strcmp(findData.cFileName, "..") != 0) { // Skip current and parent directory
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

    // Sort: directories first, then files, both alphabetically
    sort(files.begin(), files.end(), [](const FileInfo& a, const FileInfo& b) {
        if (a.isDirectory != b.isDirectory)
            return a.isDirectory > b.isDirectory;
        return a.name < b.name;
    });

    return files;
}

void drawFileList(const vector<FileInfo>& files, const string& currentPath, int selectedIndex) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    int windowHeight = csbi.srWindow.Bottom - csbi.srWindow.Top - 4; // -4 for header and footer
    int startIndex = max(0, selectedIndex - windowHeight / 2);
    if (startIndex + windowHeight >= files.size()) {
        startIndex = max(0, (int)files.size() - windowHeight);
    }

    system("cls");
    cout << "Current directory: " << currentPath << endl;
    cout << "Use Up/Down arrows to navigate, Enter to select, 'q' to return to disk selection, 'b' to go back." << endl << endl;

    for (int i = startIndex; i < min(startIndex + windowHeight, (int)files.size()); i++) {
        if (i == selectedIndex) {
            cout << ">";
        }
        else {
            cout << "   ";
        }

        cout << setw(40) << left << files[i].name;
        cout << setw(50) << left << files[i].fullPath << endl;
    }

    cout << endl << endl;
}

string getFileType(const FileInfo& file) {
    if (file.isDirectory) return "Directory";

    string extension = file.name.substr(file.name.find_last_of(".") + 1);
    transform(extension.begin(), extension.end(), extension.begin(), ::toupper);

    if (extension == "EXE") return "Application";
    if (extension == "DLL") return "Dynamic Link Library";
    if (extension == "SYS") return "System File";
    if (extension == "TXT") return "Text Document";
    if (extension == "PDF") return "PDF Document";
    if (extension == "DOC" || extension == "DOCX") return "Word Document";
    if (extension == "XLS" || extension == "XLSX") return "Excel Document";
    if (extension == "JPG" || extension == "JPEG" || extension == "PNG") return "Image";
    if (extension == "MP3" || extension == "WAV") return "Audio";
    if (extension == "MP4" || extension == "AVI") return "Video";

    return extension + " File";
}

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

string formatFileTime(const FILETIME& ft) {
    SYSTEMTIME st;
    FileTimeToSystemTime(&ft, &st);

    tm timeInfo = {};
    timeInfo.tm_year = st.wYear - 1900;
    timeInfo.tm_mon = st.wMonth - 1;
    timeInfo.tm_mday = st.wDay;
    timeInfo.tm_hour = st.wHour;
    timeInfo.tm_min = st.wMinute;
    timeInfo.tm_sec = st.wSecond;
    timeInfo.tm_isdst = -1;

    char buffer[100];
    strftime(buffer, sizeof(buffer), "%a %b %d %H:%M:%S %Y", &timeInfo);
    return string(buffer);
}

// Add this struct
struct CleanupResult {
    int deletedCount = 0;
    ULARGE_INTEGER cleanedSize = {0};
};

ULARGE_INTEGER calculateFolderSize(const string& path) {
    ULARGE_INTEGER totalSize = { 0 };
    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA((path + "\\*").c_str(), &findData);

    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (strcmp(findData.cFileName, ".") != 0 && strcmp(findData.cFileName, "..") != 0) {
                string fullPath = path + "\\" + findData.cFileName;

                if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    // Рекурсивно подсчитываем размер подпапки
                    ULARGE_INTEGER subFolderSize = calculateFolderSize(fullPath);
                    totalSize.QuadPart += subFolderSize.QuadPart;
                }
                else {
                    // Добавляем размер файла
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

// Modify cleanTempDirectory to return CleanupResult
CleanupResult cleanTempDirectory(const string& path) {
    CleanupResult result; // Initialize result for this path
    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA((path + "\\*").c_str(), &findData);

    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (strcmp(findData.cFileName, ".") != 0 && strcmp(findData.cFileName, "..") != 0) {
                string fullPath = path + "\\" + findData.cFileName;

                if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    // Recursively clean subdirectories
                    // Add results from recursive call
                    CleanupResult subResult = cleanTempDirectory(fullPath);
                    result.deletedCount += subResult.deletedCount;
                    result.cleanedSize.QuadPart += subResult.cleanedSize.QuadPart;

                    // Try to remove the directory after cleaning
                    if (RemoveDirectoryA(fullPath.c_str())) {
                        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                        cout << "Removed directory: " << fullPath << endl;
                        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
                    }
                }
                else {
                    // Try to delete the file
                    // Get file size before attempting deletion
                    ULARGE_INTEGER fileSize;
                    fileSize.LowPart = findData.nFileSizeLow;
                    fileSize.HighPart = findData.nFileSizeHigh;

                    cout << "  Attempting to delete file: " << fullPath << endl;
                    if (DeleteFileA(fullPath.c_str())) {
                        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                        cout << "  Successfully deleted file: " << fullPath << endl;
                        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
                        // Increment count and add size for deleted file
                        result.deletedCount++;
                        result.cleanedSize.QuadPart += fileSize.QuadPart;
                    }
                    else {
                        DWORD error = GetLastError();
                        if (error != ERROR_ACCESS_DENIED && error != ERROR_SHARING_VIOLATION) {
                            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_INTENSITY);
                            cout << "Failed to delete: " << fullPath << " (Error: " << error << ")" << endl;
                            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
                        }
                    }
                }
            }
        } while (FindNextFileA(hFind, &findData));
        FindClose(hFind);
    }

    return result; // Return the accumulated result
}

void showFileInfo(const FileInfo& file) {
    system("cls");
    cout << "Information for: " << file.fullPath << endl << endl;

    cout << "Type: " << getFileType(file) << endl;

    if (!file.isDirectory) {
        WIN32_FIND_DATAA findData;
        HANDLE hFind = FindFirstFileA(file.fullPath.c_str(), &findData);
        if (hFind != INVALID_HANDLE_VALUE) {
            ULARGE_INTEGER fileSize;
            fileSize.LowPart = findData.nFileSizeLow;
            fileSize.HighPart = findData.nFileSizeHigh;

            cout << "Size: " << formatFileSize(fileSize.QuadPart) << endl;
            cout << "Created: " << formatFileTime(findData.ftCreationTime) << endl;
            cout << "Modified: " << formatFileTime(findData.ftLastWriteTime) << endl;
            cout << "Accessed: " << formatFileTime(findData.ftLastAccessTime) << endl;

            // Additional attributes
            cout << "\nAttributes:" << endl;
            if (findData.dwFileAttributes & FILE_ATTRIBUTE_READONLY) cout << "- Read-only" << endl;
            if (findData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) cout << "- Hidden" << endl;
            if (findData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) cout << "- System" << endl;
            if (findData.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) cout << "- Archive" << endl;
            if (findData.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED) cout << "- Compressed" << endl;
            if (findData.dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED) cout << "- Encrypted" << endl;

            FindClose(hFind);
        }
    }
    else {
        // Directory specific information
        cout << "\nDirectory Contents:" << endl;
        int fileCount = 0, dirCount = 0;
        ULARGE_INTEGER totalSize = calculateFolderSize(file.fullPath);

        WIN32_FIND_DATAA findData;
        HANDLE hFind = FindFirstFileA((file.fullPath + "\\*").c_str(), &findData);
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                if (strcmp(findData.cFileName, ".") != 0 && strcmp(findData.cFileName, "..") != 0) {
                    if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                        dirCount++;
                    else
                        fileCount++;
                }
            } while (FindNextFileA(hFind, &findData));
            FindClose(hFind);
        }

        cout << "Size: " << formatFileSize(totalSize.QuadPart) << endl;
        cout << "Contains " << fileCount << " files and " << dirCount << " subdirectories" << endl;

        // Show creation and modification times
        WIN32_FIND_DATAA dirData;
        if (FindFirstFileA(file.fullPath.c_str(), &dirData) != INVALID_HANDLE_VALUE) {
            cout << "Created: " << formatFileTime(dirData.ftCreationTime) << endl;
            cout << "Modified: " << formatFileTime(dirData.ftLastWriteTime) << endl;
            cout << "Accessed: " << formatFileTime(dirData.ftLastAccessTime) << endl;

            cout << "\nAttributes:" << endl;
            if (dirData.dwFileAttributes & FILE_ATTRIBUTE_READONLY) cout << "- Read-only" << endl;
            if (dirData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) cout << "- Hidden" << endl;
            if (dirData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) cout << "- System" << endl;
            if (dirData.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) cout << "- Archive" << endl;
            if (dirData.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED) cout << "- Compressed" << endl;
            if (dirData.dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED) cout << "- Encrypted" << endl;
        }
    }

    cout << "\nPress any key to return..." << endl;
    _getch();
}

void renameFile(const string& oldPath) {
    system("cls");
    cout << "Rename: " << oldPath << endl;

    // Проверяем, существует ли файл
    DWORD attributes = GetFileAttributesA(oldPath.c_str());
    if (attributes == INVALID_FILE_ATTRIBUTES) {
        cout << "Error: File not found!" << endl;
        cout << "Press any key to continue...";
        _getch();
        return;
    }

    // Проверяем права доступа
    HANDLE hFile = CreateFileA(oldPath.c_str(),
                               GENERIC_READ | GENERIC_WRITE,
                               0,
                               NULL,
                               OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL,
                               NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        DWORD error = GetLastError();
        cout << "Error: Cannot access file. ";
        switch (error) {
            case ERROR_ACCESS_DENIED:
                cout << "Access denied. The file might be in use or protected." << endl;
                break;
            case ERROR_SHARING_VIOLATION:
                cout << "File is being used by another process." << endl;
                break;
            default:
                cout << "Error code: " << error << endl;
        }
        cout << "Press any key to continue...";
        _getch();
        return;
    }
    CloseHandle(hFile);

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

    // Проверяем, не существует ли уже файл с таким именем
    if (GetFileAttributesA(newPath.c_str()) != INVALID_FILE_ATTRIBUTES) {
        cout << "Error: A file with this name already exists!" << endl;
        cout << "Press any key to continue...";
        _getch();
        return;
    }

    // Пробуем переименовать с разными правами доступа
    if (!MoveFileA(oldPath.c_str(), newPath.c_str())) {
        DWORD error = GetLastError();
        cout << "Failed to rename file. ";
        switch (error) {
            case ERROR_ACCESS_DENIED:
                cout << "Access denied. Try running the program as administrator." << endl;
                break;
            case ERROR_SHARING_VIOLATION:
                cout << "File is being used by another process." << endl;
                break;
            case ERROR_ALREADY_EXISTS:
                cout << "A file with this name already exists." << endl;
                break;
            default:
                cout << "Error code: " << error << endl;
        }
    }
    else {
        cout << "File renamed successfully!" << endl;
    }

    cout << "Press any key to continue...";
    _getch();
}

void recursiveDeleteHelper(const string& path) {
    // Рекурсивное удаление директории без подтверждения
    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA((path + "\\*").c_str(), &findData);

    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (strcmp(findData.cFileName, ".") != 0 && strcmp(findData.cFileName, "..") != 0) {
                string fullPath = path + "\\" + findData.cFileName;
                cout << "  Processing: " << fullPath << endl;
                if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    recursiveDeleteHelper(fullPath); // Рекурсивно удаляем поддиректории
                    cout << "Attempting to remove directory: " << fullPath << endl;
                    if (RemoveDirectoryA(fullPath.c_str())) {
                        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                        cout << "Successfully removed directory: " << fullPath << endl;
                        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
                    } else {
                        DWORD error = GetLastError();
                        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_INTENSITY);
                        cout << "Failed to remove directory: " << fullPath << ". Error: " << error << endl;
                        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
                    }
                } else { // It's a file
                    cout << "  Attempting to delete file: " << fullPath << endl;
                    if (DeleteFileA(fullPath.c_str())) {
                        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                        cout << "  Successfully deleted file: " << fullPath << endl;
                        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
                    } else {
                        DWORD error = GetLastError();
                        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_INTENSITY);
                        cout << "  Failed to delete file: " << fullPath << ". Error: " << error << endl;
                        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
                    }
                }
            }
        } while (FindNextFileA(hFind, &findData));
        FindClose(hFind);
    }
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
                cout << "Attempting to delete directory: " << path << endl;
                recursiveDeleteHelper(path);
                // After deleting contents, attempt to remove the directory itself
                cout << "Attempting to remove directory: " << path << endl;
                if (RemoveDirectoryA(path.c_str())) {
                    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                    cout << "Directory deleted successfully!" << endl;
                    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
                } else {
                    DWORD error = GetLastError();
                    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_INTENSITY);
                    cout << "Failed to delete directory: " << path << ". Error: " << error << endl;
                    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
                }
            } else { // It's a file
                cout << "Attempting to delete file: " << path << endl;
                if (DeleteFileA(path.c_str())) {
                    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                    cout << "File deleted successfully!" << endl;
                    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
                } else {
                    DWORD error = GetLastError();
                    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_INTENSITY);
                    cout << "Failed to delete file: " << path << ". Error: " << error << endl;
                    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
                }
            }
        } else {
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_INTENSITY);
            cout << "Item not found: " << path << endl;
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        }
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

    string newPath = currentPath + "\\" + folderName;
    if (CreateDirectoryA(newPath.c_str(), NULL)) {
        cout << "Folder created successfully!" << endl;
    }
    else {
        cout << "Failed to create folder. Error: " << GetLastError() << endl;
    }
    cout << "Press any key to continue...";
    _getch();
}

void createNewFile(const string& fullPath) {
    system("cls");
    cout << "Create new file: " << fullPath << endl;

    HANDLE hFile = CreateFileA(fullPath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(hFile);
        cout << "File created successfully!" << endl;
    }
    else {
        cout << "Failed to create file. Error: " << GetLastError() << endl;
    }
    cout << "Press any key to continue...";
    _getch();
}

void showFileManagement(const FileInfo& file) {
    vector<string> options = {
            "Rename",
            "Delete",
            "Copy Path",
            "Open in Explorer",
            "Create New Folder",
            "Create New File",
            "Back"
    };
    int selectedIndex = 0;
    bool running = true;

    while (running) {
        system("cls");
        cout << "File Management: " << file.fullPath << endl << endl;

        for (int i = 0; i < options.size(); i++) {
            if (i == selectedIndex) {
                cout << "> ";
            }
            else {
                cout << "  ";
            }
            cout << options[i] << endl;
        }

        int key = _getch();
        if (g_ctrlCPressed) { // Check for Ctrl+C
            g_ctrlCPressed = FALSE; // Reset the flag
            running = false; // Exit this menu loop
            continue; // Skip the rest of the loop iteration
        }
        if (key == 224) { // Arrow key pressed
            key = _getch();
            switch (key) {
                case 72: // Up arrow
                    selectedIndex = (selectedIndex - 1 + options.size()) % options.size();
                    break;
                case 80: // Down arrow
                    selectedIndex = (selectedIndex + 1) % options.size();
                    break;
            }
        }
        else if (key == 13) { // Enter key
            switch (selectedIndex) {
                case 0: // Rename
                    renameFile(file.fullPath);
                    break;
                case 1: // Delete
                    deleteFile(file.fullPath);
                    break;
                case 2: { // Copy Path
                    if (OpenClipboard(NULL)) {
                        EmptyClipboard();
                        HGLOBAL hGlob = GlobalAlloc(GMEM_FIXED, file.fullPath.length() + 1);
                        if (hGlob) {
                            strcpy((char*)hGlob, file.fullPath.c_str());
                            SetClipboardData(CF_TEXT, hGlob);
                            GlobalFree(hGlob);
                            cout << "Path copied to clipboard!" << endl;
                        }
                        CloseClipboard();
                    }
                    cout << "Press any key to continue...";
                    _getch();
                    break;
                }
                case 3: // Open in Explorer
                    ShellExecuteA(NULL, "explore", file.fullPath.c_str(), NULL, NULL, SW_SHOW);
                    break;
                case 4: // Create New Folder
                    createNewFolder(file.isDirectory ? file.fullPath : file.fullPath.substr(0, file.fullPath.find_last_of("\\")));
                    break;
                case 5: // Create New File
                    createNewFile(file.isDirectory ? file.fullPath : file.fullPath.substr(0, file.fullPath.find_last_of("\\")));
                    break;
                case 6: // Back
                    running = false;
                    break;
            }
        }
        else if (key == 27) { // Escape key
            running = false;
        }
    }
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
    cout << "|  partialname                                      |" << endl;
    cout << "|                                                   |" << endl;
    cout << "|---------------------------------------------------|" << endl;
    cout << "\n     Press Enter to continue...";
    _getch();
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

                // Проверяем соответствие условиям поиска
                bool matches = false;
                if (exactMatch) {
                    matches = (findData.cFileName == searchPattern);
                }
                else {
                    string fileName = findData.cFileName;
                    transform(fileName.begin(), fileName.end(), fileName.begin(), ::tolower);
                    string pattern = searchPattern;
                    transform(pattern.begin(), pattern.end(), pattern.begin(), ::tolower);
                    matches = (fileName.find(pattern) != string::npos);
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

                // Рекурсивно ищем в подпапках
                if (isDirectory) {
                    try {
                        vector<FileInfo> subResults = searchFiles(fullPath, searchPattern, foldersOnly, filesOnly, exactMatch);
                        results.insert(results.end(), subResults.begin(), subResults.end());
                    }
                    catch (...) {
                        // Пропускаем папки, к которым нет доступа
                        continue;
                    }
                }
            }
        } while (FindNextFileA(hFind, &findData));
        FindClose(hFind);
    }

    return results;
}

void showSearchResults(const vector<FileInfo>& results, const string& searchPattern) {
    int selectedIndex = 0;
    bool running = true;

    while (running) {
        system("cls");
        cout << "Search Results for: " << searchPattern << endl;
        cout << "Found " << results.size() << " items" << endl;
        cout << "Use Up/Down arrows to navigate, Enter to select, 'q' to quit" << endl << endl;

        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
        int windowHeight = csbi.srWindow.Bottom - csbi.srWindow.Top - 6;
        int startIndex = max(0, selectedIndex - windowHeight / 2);
        if (startIndex + windowHeight >= results.size()) {
            startIndex = max(0, (int)results.size() - windowHeight);
        }

        for (int i = startIndex; i < min(startIndex + windowHeight, (int)results.size()); i++) {
            if (i == selectedIndex) {
                cout << ">";
            }
            else {
                cout << " ";
            }
            cout << (results[i].isDirectory ? "[DIR] " : "[FILE] ") << results[i].fullPath << endl;
        }

        int key = _getch();
        if (g_ctrlCPressed) { // Check for Ctrl+C
            g_ctrlCPressed = FALSE; // Reset the flag
            running = false; // Exit this menu loop
            continue; // Skip the rest of the loop iteration
        }
        if (key == 224) {
            key = _getch();
            switch (key) {
                case 72: // Up arrow
                    selectedIndex = (selectedIndex - 1 + results.size()) % results.size();
                    break;
                case 80: // Down arrow
                    selectedIndex = (selectedIndex + 1) % results.size();
                    break;
            }
        }
        else if (key == 13) { // Enter
            if (selectedIndex < results.size()) {
                if (results[selectedIndex].isDirectory) {
                    browseDirectory(results[selectedIndex].fullPath);
                }
                else {
                    ShellExecuteA(NULL, "open", results[selectedIndex].fullPath.c_str(), NULL, NULL, SW_SHOW);
                }
            }
        }
        else if (key == 'q' || key == 'Q') {
            running = false;
        }
    }
}

void startSearch(const string& startPath) {
    system("cls");
    cout << "Enter search pattern (type HELP:: for help): ";
    string searchPattern;
    getline(cin, searchPattern);

    if (searchPattern == "HELP::") {
        showSearchHelp();
        return;
    }
    else if (searchPattern == "q") { return; }

    bool foldersOnly = false;
    bool filesOnly = false;
    bool exactMatch = false;
    bool searchAllDrives = false;

    if (searchPattern.find("FOLDER::") == 0) {
        foldersOnly = true;
        searchPattern = searchPattern.substr(8);
    }
    else if (searchPattern.find("FILE::") == 0) {
        filesOnly = true;
        searchPattern = searchPattern.substr(6);
    }
    else if (searchPattern.find("SS::") == 0) {
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

    vector<FileInfo> results;

    if (searchAllDrives) {
        cout << "Searching across all drives... This may take a while." << endl;
        vector<DiskInfo> drives = getAvailableDrives();

        for (const auto& drive : drives) {
            cout << "Searching in " << drive.drive << "..." << endl;
            try {
                vector<FileInfo> driveResults = searchFiles(drive.drive, searchPattern, foldersOnly, filesOnly, exactMatch);
                results.insert(results.end(), driveResults.begin(), driveResults.end());
            }
            catch (...) {
                cout << "Error accessing " << drive.drive << ", skipping..." << endl;
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

void showFileManagerHelp() {
    system("cls");
    // Set console output to use UTF-8L
    SetConsoleOutputCP(CP_UTF8);
    // Enable virtual terminal processing
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    SetConsoleMode(hOut, dwMode | 0x0004);

    cout << "|-------------------------------------------------------------------------|" << endl;
    cout << "|                         Help:                                           |" << endl;
    cout << "|-------------------------------------------------------------------------|" << endl;
    cout << "| Navigation:                                                              |" << endl;
    cout << "| ↑ (Up arrow) - Move up                                                  |" << endl;
    cout << "| ↓ (Down arrow) - Move down                                              |" << endl;
    cout << "| Enter - Select                                                          |" << endl;
    cout << "| → (Right arrow) - Show detailed information                             |" << endl;
    cout << "| ← (Left arrow) - Disk management                                        |" << endl;
    cout << "|-------------------------------------------------------------------------|" << endl;
    cout << "| General commands:                                                       |" << endl;
    cout << "| 4 - Refresh file/drive list                                             |" << endl;
    cout << "| q - Quit                                                                |" << endl;
    cout << "| h - Show help                                                           |" << endl;
    cout << "| S - Search files on computer                                            |" << endl;
    cout << "| O - Additional operations                                               |" << endl;
    cout << "|-------------------------------------------------------------------------|" << endl;
    cout << "| In file manager:                                                        |" << endl;
    cout << "| b - Go back                                                             |" << endl;
    cout << "| OTHER - Additional operations                                           |" << endl;
    cout << "|-------------------------------------------------------------------------|" << endl;
    cout << "| When editing a file:                                                    |" << endl;
    cout << "| Ctrl + S - Save                                                         |" << endl;
    cout << "| Ctrl + Q - Exit without saving                                          |" << endl;
    cout << "| Ctrl + Alt + Q - Save and exit                                          |" << endl;
    cout << "|-------------------------------------------------------------------------|" << endl;
    cout << "\nPress any key to return...";
    _getch();
}

void showDiskManagement(const DiskInfo& disk) {
    vector<string> options = {
            "Check Disk",
            "Defragment",
            "Properties",
            "Back"
    };
    int selectedIndex = 0;
    bool running = true;

    while (running) {
        system("cls");
        cout << "Disk Management: " << disk.drive << endl << endl;

        for (int i = 0; i < options.size(); i++) {
            if (i == selectedIndex) {
                cout << "> ";
            }
            else {
                cout << "  ";
            }
            cout << options[i] << endl;
        }

        int key = _getch();
        if (key == 224) {
            key = _getch();
            switch (key) {
                case 72: // Up arrow
                    selectedIndex = (selectedIndex - 1 + options.size()) % options.size();
                    break;
                case 80: // Down arrow
                    selectedIndex = (selectedIndex + 1) % options.size();
                    break;
            }
        }
        else if (key == 13) { // Enter
            switch (selectedIndex) {
                case 0: // Check Disk
                    system(("chkdsk " + disk.drive + " /f").c_str());
                    cout << "\nPress any key to continue...";
                    _getch();
                    break;
                case 1: // Defragment
                    system(("defrag " + disk.drive + " /A").c_str());
                    cout << "\nPress any key to continue...";
                    _getch();
                    break;
                case 2: // Properties
                    system("cls");
                    cout << "Disk Properties: " << disk.drive << endl << endl;
                    cout << "Label: " << disk.label << endl;
                    cout << "Total Space: " << fixed << setprecision(2) << disk.totalSpace << " GB" << endl;
                    cout << "Free Space: " << fixed << setprecision(2) << disk.freeSpace << " GB" << endl;
                    cout << "Used Space: " << fixed << setprecision(2) << (disk.totalSpace - disk.freeSpace) << " GB" << endl;
                    cout << "\nPress any key to continue...";
                    _getch();
                    break;
                case 3: // Back
                    running = false;
                    break;
            }
        }
        else if (key == 27) { // Escape
            running = false;
        }
    }
}

void showAdditionalOperations(const string& currentPath) {
    vector<string> options = {
            "Create New Folder",
            "Create New File",
            "===================",
            "Copy Selected",
            "Move Selected",
            "Delete Selected",
            "Rename Selected",
            "===================",
            "Back"
    };
    int selectedIndex = 0;
    bool running = true;

    while (running) {
        system("cls");
        cout << "Additional Operations" << endl << endl;

        for (int i = 0; i < options.size(); i++) {
            // Skip separators when selecting
            if (options[i] == "===================") {
                cout << " " << options[i] << endl;
                continue;
            }

            if (i == selectedIndex) {
                cout << ">";
            }
            else {
                cout << "  ";
            }
            cout << options[i] << endl;
        }

        int key = _getch();
        if (key == 224) {
            key = _getch();
            switch (key) {
                case 72: // Up arrow
                    do {
                        selectedIndex = (selectedIndex - 1 + options.size()) % options.size();
                    } while (options[selectedIndex] == "===================");
                    break;
                case 80: // Down arrow
                    do {
                        selectedIndex = (selectedIndex + 1) % options.size();
                    } while (options[selectedIndex] == "===================");
                    break;
            }
        }
        else if (key == 13) { // Enter
            switch (selectedIndex) {
                case 0: // Create New Folder
                    createNewFolder(currentPath);
                    break;
                case 1: // Create New File
                    createNewFile(currentPath);
                    break;
                case 3: // Copy Selected
                    // TODO: Implement copy functionality
                    break;
                case 4: // Move Selected
                    // TODO: Implement move functionality
                    break;
                case 5: // Delete Selected
                    // TODO: Implement delete functionality
                    break;
                case 6: // Rename Selected
                    // TODO: Implement rename functionality
                    break;
                case 8: // Back
                    running = false;
                    break;
            }
        }
        else if (key == 27) { // Escape
            running = false;
        }
    }
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
        if (g_ctrlCPressed) { // Check for Ctrl+C
            g_ctrlCPressed = FALSE; // Reset the flag
            running = false; // Exit this menu loop
            continue; // Skip the rest of the loop iteration
        }
        if (key == 224) { // Arrow key pressed
            key = _getch();
            switch (key) {
                case 72: // Up arrow
                    selectedIndex = (selectedIndex - 1 + files.size()) % files.size();
                    needsUpdate = true;
                    break;
                case 80: // Down arrow
                    selectedIndex = (selectedIndex + 1) % files.size();
                    needsUpdate = true;
                    break;
                case 75: // Left arrow
                    if (selectedIndex < files.size()) {
                        showFileManagement(files[selectedIndex]);
                        files = getDirectoryContents(currentPath);
                        needsUpdate = true;
                    }
                    break;
                case 77: // Right arrow
                    if (selectedIndex < files.size()) {
                        showFileInfo(files[selectedIndex]);
                        needsUpdate = true;
                    }
                    break;
            }
        }
        else if (key == 13) { // Enter key
            if (selectedIndex < files.size()) {
                if (files[selectedIndex].isDirectory) {
                    if (files[selectedIndex].name == "..") {
                        // Go up one directory
                        size_t lastSlash = currentPath.find_last_of("\\");
                        if (lastSlash != string::npos) {
                            currentPath = currentPath.substr(0, lastSlash);
                            if (currentPath.empty()) currentPath = "C:\\";
                        }
                    }
                    else {
                        // Enter directory
                        currentPath = files[selectedIndex].fullPath;
                    }
                    files = getDirectoryContents(currentPath);
                    selectedIndex = 0;
                    needsUpdate = true;
                }
                else {
                    // Open file
                    ShellExecuteA(NULL, "open", files[selectedIndex].fullPath.c_str(), NULL, NULL, SW_SHOW);
                }
            }
        }
        else if (key == 'q' || key == 'Q') {
            running = false;
        }
        else if (key == 'b' || key == 'B') {
            // Go back one directory
            size_t lastSlash = currentPath.find_last_of("\\");
            if (lastSlash != string::npos) {
                currentPath = currentPath.substr(0, lastSlash);
                if (currentPath.empty()) currentPath = "C:\\";
                files = getDirectoryContents(currentPath);
                selectedIndex = 0;
                needsUpdate = true;
            }
        }
        else if (key == 's' || key == 'S') {
            startSearch(currentPath);
            files = getDirectoryContents(currentPath);
            needsUpdate = true;
        }
        else if (key == 'h' || key == 'H') {
            showFileManagerHelp();
            needsUpdate = true;
        }
        else if (key == '4') {
            files = getDirectoryContents(currentPath);
            needsUpdate = true;
        }
        else if (key == 'o' || key == 'O') {
            showAdditionalOperations(currentPath);
            files = getDirectoryContents(currentPath);
            needsUpdate = true;
        }
    }
}

bool checkFileAccess(const string& filePath) {
    HANDLE hFile = CreateFileA(
            filePath.c_str(),
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        DWORD error = GetLastError();
        cout << "Error accessing file: " << filePath << endl;
        cout << "Error code: " << error << endl;

        switch (error) {
            case ERROR_ACCESS_DENIED:
                cout << "Access denied. Try running the program as administrator." << endl;
                break;
            case ERROR_SHARING_VIOLATION:
                cout << "File is being used by another process." << endl;
                break;
            case ERROR_FILE_NOT_FOUND:
                cout << "File not found." << endl;
                break;
            default:
                cout << "Unknown error occurred." << endl;
        }

        return false;
    }

    CloseHandle(hFile);
    return true;
}

void showFileAccessError() {
    system("cls");
    cout << "+-------------------------------------------------------------+" << endl;
    cout << "|                   File Access Error                         |" << endl;
    cout << "+-------------------------------------------------------------+" << endl;
    cout << "| Possible causes:                                            |" << endl;
    cout << "| 1. Program is still running                                 |" << endl;
    cout << "| 2. Antivirus is blocking access                             |" << endl;
    cout << "| 3. Insufficient access rights                               |" << endl;
    cout << "+-------------------------------------------------------------+" << endl;
    cout << "| Solutions:                                                  |" << endl;
    cout << "| 1. Close all instances of the program                       |" << endl;
    cout << "| 2. Run the program as administrator                         |" << endl;
    cout << "| 3. Temporarily disable antivirus                            |" << endl;
    cout << "| 4. Restart the computer                                     |" << endl;
    cout << "+-------------------------------------------------------------+" << endl;
    cout << "\nPress any key to continue...";
    _getch();
}

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
        if (g_ctrlCPressed) { // Check for Ctrl+C
            g_ctrlCPressed = FALSE; // Reset the flag
            running = false; // Exit this menu loop
            continue; // Skip the rest of the loop iteration
        }
        if (key == 224) { // Arrow key pressed
            key = _getch();
            switch (key) {
                case 72: // Up arrow
                    selectedIndex = (selectedIndex - 1 + drives.size()) % drives.size();
                    needsUpdate = true;
                    break;
                case 80: // Down arrow
                    selectedIndex = (selectedIndex + 1) % drives.size();
                    needsUpdate = true;
                    break;
                case 75: // Left arrow
                    // Show disk management options
                    system("cls");
                    cout << "Disk Management for " << drives[selectedIndex].drive << endl;
                    cout << "1. Check Disk" << endl;
                    cout << "2. Defragment" << endl;
                    cout << "3. Properties" << endl;
                    cout << "Press any key to return..." << endl;
                    _getch();
                    needsUpdate = true;
                    break;
                case 77: // Right arrow
                    // Show detailed disk info
                    system("cls");
                    cout << "Detailed Information for " << drives[selectedIndex].drive << endl;
                    cout << "Label: " << drives[selectedIndex].label << endl;
                    cout << "Free Space: " << fixed << setprecision(2) << drives[selectedIndex].freeSpace << " GB" << endl;
                    cout << "Total Space: " << fixed << setprecision(2) << drives[selectedIndex].totalSpace << " GB" << endl;
                    cout << "Press any key to return..." << endl;
                    _getch();
                    needsUpdate = true;
                    break;
            }
        }
        else if (key == 13) { browseDirectory(drives[selectedIndex].drive); needsUpdate = true; }
        else if (key == 'q' || key == 'Q') { running = false; }
        else if (key == 'h' || key == 'H') { showFileManagerHelp(); needsUpdate = true; }
        else if (key == '4') { drives = getAvailableDrives(); needsUpdate = true; }
        else if (key == 's' || key == 'S') { startSearch(""); needsUpdate = true; }
    }
}

// Add this function before main()


// Helper function to convert FILETIME to ULONGLONG
ULONGLONG FileTimeToULONGLONG(const FILETIME& ft) {
    ULARGE_INTEGER ul;
    ul.LowPart = ft.dwLowDateTime;
    ul.HighPart = ft.dwHighDateTime;
    return ul.QuadPart;
}

// Helper function to clear a specific number of lines from the current cursor position
void clearConsoleLines(int numLines) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    DWORD written;
    COORD coord = csbi.dwCursorPosition;

    // Move to the beginning of the line and clear the line
    for (int i = 0; i < numLines; ++i) {
        coord.Y = csbi.dwCursorPosition.Y + i;
        coord.X = 0;
        FillConsoleOutputCharacter(hConsole, ' ', csbi.dwSize.X, coord, &written);
        FillConsoleOutputAttribute(hConsole, csbi.wAttributes, csbi.dwSize.X, coord, &written);
    }
    // Restore cursor position
    SetConsoleCursorPosition(hConsole, csbi.dwCursorPosition);
}

// Custom Terminal Function
void customTerminal() {
    system("cls");
    cout << "=== Custom Terminal (Bash-like) ===" << endl;
    cout << "Type 'exit' to return to main menu" << endl;
    cout << "Type 'help' for available commands" << endl;
    cout << "===================================" << endl << endl;

    string currentDir = "C:\\";
    vector<string> commandHistory;
    int historyIndex = -1;

    while (true) {
        // Get current directory
        char buffer[MAX_PATH];
        GetCurrentDirectoryA(MAX_PATH, buffer);
        currentDir = string(buffer);

        // Display prompt
        cout << "[" << currentDir << "] $ ";

        // Get user input
        string command;
        getline(cin, command);

        // Handle empty command
        if (command.empty()) {
            continue;
        }

        // Add to history
        commandHistory.push_back(command);
        historyIndex = commandHistory.size();

        // Parse command
        string cmd = command;
        transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);

        // Handle built-in commands
        if (cmd == "exit" || cmd == "quit") {
            break;
        }
        else if (cmd == "help") {
            cout << "\nAvailable commands:" << endl;
            cout << "  ls, dir          - List directory contents" << endl;
            cout << "  cd <path>        - Change directory" << endl;
            cout << "  pwd              - Print working directory" << endl;
            cout << "  mkdir <name>     - Create directory" << endl;
            cout << "  rm <file>        - Remove file" << endl;
            cout << "  rmdir <dir>      - Remove directory" << endl;
            cout << "  cp <src> <dst>   - Copy file" << endl;
            cout << "  mv <src> <dst>   - Move/rename file" << endl;
            cout << "  cat <file>       - Display file contents" << endl;
            cout << "  echo <text>      - Print text" << endl;
            cout << "  clear            - Clear screen" << endl;
            cout << "  date             - Show current date/time" << endl;
            cout << "  whoami           - Show current user" << endl;
            cout << "  hostname         - Show computer name" << endl;
            cout << "  systeminfo       - Show system information" << endl;
            cout << "  processes        - Show running processes" << endl;
            cout << "  services         - Show running services" << endl;
            cout << "  netstat          - Show network connections" << endl;
            cout << "  ipconfig         - Show network configuration" << endl;
            cout << "  encrypt -pass <pwd> <file> - Encrypt file with password" << endl;
            cout << "  deencrypt [file] - Decrypt file (will prompt for password)" << endl;
            cout << "  create_hash      - Generate random hashes with flags: -l, -q, -s, -f, -h" << endl;
            cout << "  network          - Network management and testing menu" << endl;
            cout << "  help             - Show this help" << endl;
            cout << "  exit, quit       - Exit terminal" << endl;
            cout << "  Any other command will be executed as Windows command" << endl << endl;
        }
        else if (cmd == "clear" || cmd == "cls") {
            system("cls");
            cout << "=== Custom Terminal (Bash-like) ===" << endl;
            cout << "Type 'exit' to return to main menu" << endl;
            cout << "Type 'help' for available commands" << endl;
            cout << "===================================" << endl << endl;
        }
        else if (cmd == "pwd") {
            cout << currentDir << endl;
        }
        else if (cmd.substr(0, 2) == "cd") {
            string path = command.substr(2);
            if (!path.empty() && path[0] == ' ') {
                path = path.substr(1);
            }
            if (path.empty() || path == "~") {
                path = "C:\\";
            }
            if (SetCurrentDirectoryA(path.c_str())) {
                cout << "Changed directory to: " << path << endl;
            } else {
                cout << "Error: Cannot change to directory " << path << endl;
            }
        }
        else if (cmd == "ls" || cmd == "dir") {
            string dirCommand = "dir";
            system(dirCommand.c_str());
        }
        else if (cmd.substr(0, 5) == "mkdir") {
            string dirName = command.substr(5);
            if (!dirName.empty() && dirName[0] == ' ') {
                dirName = dirName.substr(1);
            }
            if (!dirName.empty()) {
                string mkdirCommand = "mkdir \"" + dirName + "\"";
                system(mkdirCommand.c_str());
            } else {
                cout << "Error: Directory name required" << endl;
            }
        }
        else if (cmd.substr(0, 2) == "rm") {
            string fileName = command.substr(2);
            if (!fileName.empty() && fileName[0] == ' ') {
                fileName = fileName.substr(1);
            }
            if (!fileName.empty()) {
                string delCommand = "del \"" + fileName + "\"";
                system(delCommand.c_str());
            } else {
                cout << "Error: File name required" << endl;
            }
        }
        else if (cmd.substr(0, 5) == "rmdir") {
            string dirName = command.substr(5);
            if (!dirName.empty() && dirName[0] == ' ') {
                dirName = dirName.substr(1);
            }
            if (!dirName.empty()) {
                string rmdirCommand = "rmdir \"" + dirName + "\"";
                system(rmdirCommand.c_str());
            } else {
                cout << "Error: Directory name required" << endl;
            }
        }
        else if (cmd.substr(0, 2) == "cp") {
            string args = command.substr(2);
            if (!args.empty() && args[0] == ' ') {
                args = args.substr(1);
            }
            if (!args.empty()) {
                string copyCommand = "copy " + args;
                system(copyCommand.c_str());
            } else {
                cout << "Error: Source and destination required" << endl;
            }
        }
        else if (cmd.substr(0, 2) == "mv") {
            string args = command.substr(2);
            if (!args.empty() && args[0] == ' ') {
                args = args.substr(1);
            }
            if (!args.empty()) {
                string moveCommand = "move " + args;
                system(moveCommand.c_str());
            } else {
                cout << "Error: Source and destination required" << endl;
            }
        }
        else if (cmd.substr(0, 3) == "cat") {
            string fileName = command.substr(3);
            if (!fileName.empty() && fileName[0] == ' ') {
                fileName = fileName.substr(1);
            }
            if (!fileName.empty()) {
                string typeCommand = "type \"" + fileName + "\"";
                system(typeCommand.c_str());
            } else {
                cout << "Error: File name required" << endl;
            }
        }
        else if (cmd.substr(0, 4) == "echo") {
            string text = command.substr(4);
            if (!text.empty() && text[0] == ' ') {
                text = text.substr(1);
            }
            cout << text << endl;
        }
        else if (cmd == "date") {
            system("date /t");
            system("time /t");
        }
        else if (cmd == "whoami") {
            system("whoami");
        }
        else if (cmd == "hostname") {
            system("hostname");
        }
        else if (cmd == "systeminfo") {
            system("systeminfo");
        }
        else if (cmd == "processes") {
            system("tasklist");
        }
        else if (cmd == "services") {
            system("net start");
        }
        else if (cmd == "netstat") {
            system("netstat -an");
        }
        else if (cmd == "ipconfig") {
            system("ipconfig");
        }
        else if (cmd.substr(0, 11) == "create_hash") {
            // Parse create_hash command
            string args = command.substr(11);
            if (!args.empty() && args[0] == ' ') {
                args = args.substr(1);
            }

            // Parse arguments into vector
            vector<string> tokens;
            stringstream ss(args);
            string token;
            while (ss >> token) {
                tokens.push_back(token);
            }

            // Use the function from terminal_commands module
            createHash(tokens);
        }
        else if (cmd == "network") {
            // Use the function from terminal_commands module
            showNetworkMenu();
        }
        else {
            // Execute as Windows command
            system(command.c_str());
        }

        cout << endl;
    }

    system("cls");
    cout << "Returning to main menu..." << endl;
    Sleep(100);
}



int main() {
    // Check if running as administrator
    BOOL isAdmin = FALSE;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID AdministratorsGroup;

    if (AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                 DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &AdministratorsGroup)) {
        if (!CheckTokenMembership(NULL, AdministratorsGroup, &isAdmin)) {
            isAdmin = FALSE;
        }
        FreeSid(AdministratorsGroup);
    }

    if (!isAdmin) {
        cout << "The program must be run as administrator!" << endl;
        cout << "Press any key to exit...";
        _getch();
        return 1;
    }

    SetConsoleTitleA("Menu System");
    hideCursor();

    // Register console control handler
    if (!SetConsoleCtrlHandler(CtrlHandler, TRUE)) {
        // Handle error if registration fails
        cout << "Error: Failed to register Ctrl+C handler!" << endl;
        // Decide whether to exit or continue
    }

    vector<string> menuItems = {
            "File Manager",
            "Check Startup",
            "Users\n",
            "Clear TEMP Files",
            "System Info\n",
            "CMD",
            "POWERSHELL\n",
            "Help",
            "Exit"
    };

    int selectedIndex = 0;
    bool running = true;

    while (running) {
        drawMenu(menuItems, selectedIndex);

        int key = _getch();
        if (g_ctrlCPressed) { // Check for Ctrl+C
            g_ctrlCPressed = FALSE; // Reset the flag
            running = false; // Exit the main menu loop
            continue; // Skip the rest of the loop iteration
        }
        if (key == 224) { // Arrow key pressed
            key = _getch();
            switch (key) {
                case 72: // Up arrow
                    selectedIndex = (selectedIndex - 1 + menuItems.size()) % menuItems.size();
                    break;
                case 80: // Down arrow
                    selectedIndex = (selectedIndex + 1) % menuItems.size();
                    break;
                case 77: // Right arrow - Open custom terminal
                    customTerminal();
                    break;
            }
        }
        else if (key == 13) { // Enter key
            switch (selectedIndex) {
                case 0: // File Manager
                    fileManager();
                    break;
                case 1: // Check Startup
                    showStartupLocationsMenu();
                    break;
                case 2: // Users
                    system("cls");
                    cout << "Listing Users:" << endl << endl;
                    system("net user");
                    cout << "\nPress any key to continue...";
                    _getch();
                    break;
                case 3: // Clear Temp File
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
                    break;
                }
                case 4: // System Info
                    system("cls");
                    cout << "System Information:" << endl << endl;
                    cout << "Windows Version: ";
                    system("ver");
                    cout << "\nComputer Name: ";
                    system("hostname");
                    cout << "\nPress any key to continue...";
                    _getch();
                    break;
                case 5: // CMD
                    system("cls");
                    system("cmd");
                    _getch();
                    break;
                case 6: // POWERSHELL
                    system("cls");
                    system("powershell");
                    _getch();
                    break;
                case 7: // Help
                    showHelp();
                    break;
                case 8: // Exit
                    running = false;
                    break;
            }
        }
        else if (key == 'h' || key == 'H') {
            showHelp();
        }
        else if (key == 'q' || key == 'Q') {
            running = false;
        }
    }

    showCursor();
    return 0;
}

void showStartupLocationsMenu() {
    vector<string> options = {
            "Startup Folder",
            "Task Scheduler",
            "Registry",
            "Shell/Userinit\n",
            "Restore to original\n",
            "Back to Main Menu"
    };
    int selectedIndex = 0;
    bool running = true;

    while (running) {
        system("cls");
        cout << "Check Startup Locations" << endl;
        cout << "Use Up and Down arrows to navigate, Enter to select, 'q' to quit" << endl << endl;

        for (int i = 0; i < options.size(); i++) {
            if (i == selectedIndex) {
                cout << ">";
            }
            else {
                cout << "   ";
            }

            // Set color for "Restore to original" option
            if (options[i] == "Restore to original\n") {
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_INTENSITY);
                cout << options[i] << endl;
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            } else {
                cout << options[i] << endl;
            }
        }

        int key = _getch();
        if (g_ctrlCPressed) {
            g_ctrlCPressed = FALSE;
            running = false;
            continue;
        }
        if (key == 224) {
            key = _getch();
            switch (key) {
                case 72: // Up arrow
                    selectedIndex = (selectedIndex - 1 + options.size()) % options.size();
                    break;
                case 77: // Right arrow - Show ALL Startup
                        SHOW_ALL_STARTUP();
                    break;
                case 80: // Down arrow
                    selectedIndex = (selectedIndex + 1) % options.size();
                    break;
            }
        }
        else if (key == 13) { // Enter
            switch (selectedIndex) {
                case 0: // Startup Folder
                    checkStartupFolder();
                    break;
                case 1: // Task Scheduler
                    checkTaskScheduler();
                    break;
                case 2: // Registry
                    checkRegistryStartup();
                    break;
                case 3: // Shell/Userinit
                    checkShellUserinit();
                    break;
                case 4: // Restore to original
                    restoreStartupSettings();
                    break;
                case 5: // Back to Main Menu
                    running = false;
                    break;
            }
        }
        else if (key == 'q' || key == 'Q') {
            running = false;
        }
    }
}


void checkStartupFolder() {
    system("cls");
    cout << "========================================" << endl;
    cout << "   Startup Folder Check" << endl;
    cout << "========================================" << endl << endl;

    cout << "Checking Startup Folders..." << endl << endl;
    writeLog("User accessed Startup Folder check");

    // Current User Startup Folder
    cout << "Current User Startup Folder:" << endl;
    cout << "-----------------------------" << endl;
    char appDataPath[MAX_PATH];
    if (SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, appDataPath) == S_OK) {
        string startupPath = string(appDataPath) + "\\Microsoft\\Windows\\Start Menu\\Programs\\Startup";
        if (GetFileAttributesA(startupPath.c_str()) != INVALID_FILE_ATTRIBUTES) {
            cout << "✓ Found: " << startupPath << endl << endl;
            cout << "Contents:" << endl;

            WIN32_FIND_DATAA findData;
            HANDLE hFind = FindFirstFileA((startupPath + "\\*").c_str(), &findData);
            if (hFind != INVALID_HANDLE_VALUE) {
                bool hasFiles = false;
                do {
                    if (strcmp(findData.cFileName, ".") != 0 && strcmp(findData.cFileName, "..") != 0) {
                        cout << "  " << findData.cFileName << endl;
                        hasFiles = true;
                    }
                } while (FindNextFileA(hFind, &findData));
                FindClose(hFind);
                if (!hasFiles) {
                    cout << "  (Empty)" << endl;
                }
            }
        } else {
            cout << "✗ Not found: " << startupPath << endl;
        }
    }

    cout << endl;

    // All Users Startup Folder
    cout << "All Users Startup Folder:" << endl;
    cout << "-------------------------" << endl;
    char programDataPath[MAX_PATH];
    if (SHGetFolderPathA(NULL, CSIDL_COMMON_APPDATA, NULL, 0, programDataPath) == S_OK) {
        string commonStartupPath = string(programDataPath) + "\\Microsoft\\Windows\\Start Menu\\Programs\\Startup";
        if (GetFileAttributesA(commonStartupPath.c_str()) != INVALID_FILE_ATTRIBUTES) {
            cout << "✓ Found: " << commonStartupPath << endl << endl;
            cout << "Contents:" << endl;

            WIN32_FIND_DATAA findData;
            HANDLE hFind = FindFirstFileA((commonStartupPath + "\\*").c_str(), &findData);
            if (hFind != INVALID_HANDLE_VALUE) {
                bool hasFiles = false;
                do {
                    if (strcmp(findData.cFileName, ".") != 0 && strcmp(findData.cFileName, "..") != 0) {
                        cout << "  " << findData.cFileName << endl;
                        hasFiles = true;
                    }
                } while (FindNextFileA(hFind, &findData));
                FindClose(hFind);
                if (!hasFiles) {
                    cout << "  (Empty)" << endl;
                }
            }
        } else {
            cout << "✗ Not found: " << commonStartupPath << endl;
        }
    }

    cout << endl;
    cout << "------------------------------------------------------------------------------------------------------------------------" << endl;
    cout << "Commands:" << endl;
    cout << "open_user_startup    - Open current user startup folder" << endl;
    cout << "open_all_startup     - Open all users startup folder" << endl;
    cout << "q (back)" << endl;
    cout << "Enter command: ";

    string command;
    getline(cin, command);

    if (command == "q" || command == "Q") {
        return;
    }
    else if (command == "open_user_startup") {
        char appDataPath[MAX_PATH];
        if (SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, appDataPath) == S_OK) {
            string startupPath = string(appDataPath) + "\\Microsoft\\Windows\\Start Menu\\Programs\\Startup";
            ShellExecuteA(NULL, "explore", startupPath.c_str(), NULL, NULL, SW_SHOW);
            cout << "Opened user startup folder" << endl;
        }
    }
    else if (command == "open_all_startup") {
        char programDataPath[MAX_PATH];
        if (SHGetFolderPathA(NULL, CSIDL_COMMON_APPDATA, NULL, 0, programDataPath) == S_OK) {
            string commonStartupPath = string(programDataPath) + "\\Microsoft\\Windows\\Start Menu\\Programs\\Startup";
            ShellExecuteA(NULL, "explore", commonStartupPath.c_str(), NULL, NULL, SW_SHOW);
            cout << "Opened all users startup folder" << endl;
        }
    }
    else if (!command.empty()) {
        cout << "Unknown command. Available commands:" << endl;
        cout << "open_user_startup    - Open current user startup folder" << endl;
        cout << "open_all_startup     - Open all users startup folder" << endl;
        cout << "q (back)" << endl;
    }

    cout << endl << "Press any key to continue...";
    _getch();
}

void checkTaskScheduler() {
    system("cls");
    system("schtasks /query /fo table 2>nul");
    cout << endl;

    cout << "------------------------------------------------------------------------------------------------------------------------" << endl;
    cout << "Commands:" << endl;
    cout << "open_taskschd        - Open Task Scheduler MMC" << endl;
    cout << "q (back)" << endl;
    cout << "Enter command: ";

    string command;
    getline(cin, command);

    if (command == "q" || command == "Q") {
        return;
    }
    else if (command == "open_taskschd") {
        system("taskschd.msc");
        cout << "Opened Task Scheduler" << endl;
    }
    else if (!command.empty()) {
        cout << "Unknown command. Available commands:" << endl;
        cout << "open_taskschd        - Open Task Scheduler MMC" << endl;
        cout << "list_all_tasks       - List all scheduled tasks" << endl;
        cout << "q (back)" << endl;
    }

    cout << endl << "Press any key to continue...";
    _getch();
}

void checkRegistryStartup() {
    system("cls");
    cout << "========================================" << endl;
    cout << "   Registry Startup Check" << endl;
    cout << "========================================" << endl << endl;

    cout << "Checking Registry Startup Locations..." << endl << endl;
    cout << "Current Registry Entries:" << endl << endl;
    writeLog("User accessed Registry Startup check");

    // Check HKCU Run
//    cout << "Checking: HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run" << endl;
    system("reg query \"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Run\" 2>nul");
    cout << endl;

    // Check HKCU RunOnce
//    cout << "Checking: HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce" << endl;
    system("reg query \"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce\" 2>nul");
    cout << endl;

    // Check HKLM Run
//    cout << "Checking: HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run" << endl;
    system("reg query \"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Run\" 2>nul");
    cout << endl;

    // Check HKLM RunOnce
//    cout << "Checking: HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce" << endl;
    system("reg query \"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce\" 2>nul");
    cout << endl;

    cout << "------------------------------------------------------------------------------------------------------------------------" << endl;
    cout << "Commands:" << endl;
    cout << "addhkcu_run <name> <value>" << endl;
    cout << "addhklm_run <name> <value>" << endl;
    cout << "deletehkcu_run <name>" << endl;
    cout << "deletehklm_run <name>" << endl;
    cout << "q (back)" << endl;
    cout << "Enter command: ";

    string command;
    getline(cin, command);

    if (command == "q" || command == "Q") {
        return;
    }

    // Parse and execute commands
    stringstream ss(command);
    string cmd, name, value;
    ss >> cmd;

    if (cmd == "addhkcu_run") {
        ss >> name;
        getline(ss, value);
        if (!name.empty() && !value.empty()) {
            string regCommand = "reg add \"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Run\" /v \"" + name + "\" /t REG_SZ /d \"" + value + "\" /f";
            system(regCommand.c_str());
            cout << "Added to HKCU Run: " << name << endl;
            writeLog("Added registry entry HKCU Run: " + name + " = " + value);
        } else {
            cout << "Usage: addhkcu_run <name> <value>" << endl;
        }
    }
    else if (cmd == "addhklm_run") {
        ss >> name;
        getline(ss, value);
        if (!name.empty() && !value.empty()) {
            string regCommand = "reg add \"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Run\" /v \"" + name + "\" /t REG_SZ /d \"" + value + "\" /f";
            system(regCommand.c_str());
            cout << "Added to HKLM Run: " << name << endl;
            writeLog("Added registry entry HKLM Run: " + name + " = " + value);
        } else {
            cout << "Usage: addhklm_run <name> <value>" << endl;
        }
    }
    else if (cmd == "deletehkcu_run") {
        ss >> name;
        if (!name.empty()) {
            string regCommand = "reg delete \"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Run\" /v \"" + name + "\" /f";
            system(regCommand.c_str());
            cout << "Deleted from HKCU Run: " << name << endl;
            writeLog("Deleted registry entry HKCU Run: " + name);
        } else {
            cout << "Usage: deletehkcu_run <name>" << endl;
        }
    }
    else if (cmd == "deletehklm_run") {
        ss >> name;
        if (!name.empty()) {
            string regCommand = "reg delete \"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Run\" /v \"" + name + "\" /f";
            system(regCommand.c_str());
            cout << "Deleted from HKLM Run: " << name << endl;
            writeLog("Deleted registry entry HKLM Run: " + name);
        } else {
            cout << "Usage: deletehklm_run <name>" << endl;
        }
    }
    else if (!command.empty()) {
        cout << "Unknown command. Available commands:" << endl;
        cout << "addhkcu_run <name> <value>" << endl;
        cout << "addhklm_run <name> <value>" << endl;
        cout << "deletehkcu_run <name>" << endl;
        cout << "deletehklm_run <name>" << endl;
        cout << "q (back)" << endl;
    }

    cout << endl << "Press any key to continue...";
    _getch();
}

void SHOW_ALL_STARTUP() {
    system("cls");
    string user_input;
    cout << "1: show all startup\n";
    cout << "2: show/edit startup\n";
    cout << "\ninput: ";
    cin >> user_input;
    if ( user_input == "1" ) {
        SHOW_ALL_STARTUP_2();
    } else if ( user_input == "2" ) {
        SHOW_ALL_STARTUP_1();
    } else {
        cout << "error input:";
        return SHOW_ALL_STARTUP();
    }
}

void SHOW_ALL_STARTUP_1() {
    startupMG_SHOW_ALL_STARTUP();
}

void SHOW_ALL_STARTUP_2() {
    system("cls");
    cout << "========================================" << endl;
    cout << "    SHOW ALL STARTUP" << endl;
    cout << "========================================" << endl << endl;

    // Folder startup
    cout << "Current User Startup Folder:" << endl;
    cout << "-----------------------------" << endl;
    char appDataPath[MAX_PATH];
    if (SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, appDataPath) == S_OK) {
        string startupPath = string(appDataPath) + "\\Microsoft\\Windows\\Start Menu\\Programs\\Startup";
        if (GetFileAttributesA(startupPath.c_str()) != INVALID_FILE_ATTRIBUTES) {
            cout << "✓ Found: " << startupPath << endl << endl;
            cout << "Contents:" << endl;

            WIN32_FIND_DATAA findData;
            HANDLE hFind = FindFirstFileA((startupPath + "\\*").c_str(), &findData);
            if (hFind != INVALID_HANDLE_VALUE) {
                bool hasFiles = false;
                do {
                    if (strcmp(findData.cFileName, ".") != 0 && strcmp(findData.cFileName, "..") != 0) {
                        cout << "  " << findData.cFileName << endl;
                        hasFiles = true;
                    }
                } while (FindNextFileA(hFind, &findData));
                FindClose(hFind);
                if (!hasFiles) {
                    cout << "  (Empty)" << endl;
                }
            }
        } else {
            cout << "✗ Not found: " << startupPath << endl;
        }
    }

    cout << endl;

    // All Users Startup Folder
    cout << "All Users Startup Folder:" << endl;
    cout << "-------------------------" << endl;
    char programDataPath[MAX_PATH];
    if (SHGetFolderPathA(NULL, CSIDL_COMMON_APPDATA, NULL, 0, programDataPath) == S_OK) {
        string commonStartupPath = string(programDataPath) + "\\Microsoft\\Windows\\Start Menu\\Programs\\Startup";
        if (GetFileAttributesA(commonStartupPath.c_str()) != INVALID_FILE_ATTRIBUTES) {
            cout << "✓ Found: " << commonStartupPath << endl << endl;
            cout << "Contents:" << endl;

            WIN32_FIND_DATAA findData;
            HANDLE hFind = FindFirstFileA((commonStartupPath + "\\*").c_str(), &findData);
            if (hFind != INVALID_HANDLE_VALUE) {
                bool hasFiles = false;
                do {
                    if (strcmp(findData.cFileName, ".") != 0 && strcmp(findData.cFileName, "..") != 0) {
                        cout << "  " << findData.cFileName << endl;
                        hasFiles = true;
                    }
                } while (FindNextFileA(hFind, &findData));
                FindClose(hFind);
                if (!hasFiles) {
                    cout << "  (Empty)" << endl;
                }
            }
        } else {
            cout << "✗ Not found: " << commonStartupPath << endl;
        }
    }


    // Check HKCU Run
    system("reg query \"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Run\" 2>nul");
    cout << endl;

    // Check HKCU RunOnce
    system("reg query \"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce\" 2>nul");
    cout << endl;

    // Check HKLM Run
    system("reg query \"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Run\" 2>nul");
    cout << endl;

    // Check HKLM RunOnce
    system("reg query \"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce\" 2>nul");
    cout << endl;


    auto showRegistryValues = [](const string& regPath, const string& description = "") {
        cout << "• " << regPath;
        if (!description.empty()) {
            cout << " (" << description << ")";
        }
        cout << endl;

        // Создаем временный файл для вывода reg query
        string tempFile = "temp_reg_output.txt";
        string command = "reg query \"" + regPath + "\" 2>nul > " + tempFile;
        system(command.c_str());

        // Читаем и выводим содержимое
        ifstream file(tempFile);
        string line;
        bool hasContent = false;

        while (getline(file, line)) {
            if (!line.empty() && line.find(regPath) == string::npos) {
                if (!hasContent) {
                    cout << "\t\tCurrent values:" << endl;
                    hasContent = true;
                }
                cout << "\t\t  " << line << endl;
            }
        }
        file.close();

        // Удаляем временный файл
        remove(tempFile.c_str());

        if (!hasContent) {
            cout << "\t\t(No values found)" << endl;
        }
        cout << endl;
    };
    auto showRegistryKeyValues = [](const string& regPath, const string& valueName = "") {
        cout << "• " << regPath;
        if (!valueName.empty()) {
            cout << "\\" << valueName;
        }
        cout << endl;

        string tempFile = "temp_reg_output.txt";
        string command;
        if (valueName.empty()) {
            command = "reg query \"" + regPath + "\" 2>nul > " + tempFile;
        } else {
            command = "reg query \"" + regPath + "\" /v \"" + valueName + "\" 2>nul > " + tempFile;
        }
        system(command.c_str());

        ifstream file(tempFile);
        string line;
        bool hasContent = false;

        while (getline(file, line)) {
            if (!line.empty() && line.find(regPath) == string::npos) {
                if (!hasContent) {
                    cout << "\t\tCurrent value: ";
                    hasContent = true;
                }
                cout << line << endl;
            }
        }
        file.close();

        remove(tempFile.c_str());

        if (!hasContent) {
            cout << "\t\t(Value not set)" << endl;
        }
        cout << endl;
    };

    showRegistryValues("HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", "User startup programs");
    showRegistryValues("HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", "Machine startup programs");
    showRegistryValues("HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce", "User run once programs");
    showRegistryValues("HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce", "Machine run once programs");
    showRegistryValues("HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\RunServices", "User services");
    showRegistryValues("HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\RunServices", "Machine services");
    showRegistryValues("HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\RunServicesOnce", "User run once services");
    showRegistryValues("HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\RunServicesOnce", "Machine run once services");
    showRegistryValues("HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ShellExecuteHooks", "Shell execute hooks");
    showRegistryValues("HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\SharedTaskScheduler", "Shared tasks");
    showRegistryValues("HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Browser Helper Objects", "Browser helpers");
    showRegistryValues("HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\Run", "Machine policies");
    showRegistryValues("HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\Run", "User policies");
    showRegistryValues("HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\Run", "Machine approved startup");
    showRegistryValues("HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\Run", "User approved startup");
    showRegistryValues("HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\Run32", "Machine 32-bit approved");
    showRegistryValues("HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\Run32", "User 32-bit approved");
    showRegistryValues("HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\StartupFolder", "Machine startup folder");
    showRegistryValues("HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\StartupFolder", "User startup folder");
    showRegistryValues("HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\StartupFolderCommon", "Machine common startup");
    showRegistryValues("HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\StartupFolderCommon", "User common startup");
    showRegistryValues("HKLM\\SYSTEM\\CurrentControlSet\\Services", "System services");

    cout << "WINLOGON VALUES:" << endl;
    cout << "----------------" << endl;
    showRegistryKeyValues("HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon", "Taskman");
    showRegistryKeyValues("HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon", "AppSetup");
    showRegistryKeyValues("HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon", "System");

    cout << "TASK SCHEDULER TASKS:" << endl;
    cout << "---------------------" << endl;
    cout << "• All tasks containing 'startup' in name" << endl;
    // Показ задач планировщика
    system("schtasks /query /fo list | findstr /i \"startup\" > temp_tasks.txt 2>nul");
    ifstream taskFile("temp_tasks.txt");
    string taskLine;
    bool hasTasks = false;
    cout << "\t\tCurrent tasks:" << endl;
    while (getline(taskFile, taskLine)) {
        if (!taskLine.empty()) {
            cout << "\t\t  " << taskLine << endl;
            hasTasks = true;
        }
    }
    taskFile.close();
    remove("temp_tasks.txt");
    if (!hasTasks) {
        cout << "\t\t  (No startup tasks found)" << endl;
    }
    cout << endl;

    cout << "• All tasks containing 'logon' in name" << endl;
    system("schtasks /query /fo list | findstr /i \"logon\" > temp_tasks.txt 2>nul");
    taskFile.open("temp_tasks.txt");
    hasTasks = false;
    cout << "\t\tCurrent tasks:" << endl;
    while (getline(taskFile, taskLine)) {
        if (!taskLine.empty()) {
            cout << "\t\t  " << taskLine << endl;
            hasTasks = true;
        }
    }
    taskFile.close();
    remove("temp_tasks.txt");
    if (!hasTasks) {
        cout << "\t\t  (No logon tasks found)" << endl;
    }
    cout << endl;

    cout << "• All tasks containing 'boot' in name" << endl;
    system("schtasks /query /fo list | findstr /i \"boot\" > temp_tasks.txt 2>nul");
    taskFile.open("temp_tasks.txt");
    hasTasks = false;
    cout << "\t\tCurrent tasks:" << endl;
    while (getline(taskFile, taskLine)) {
        if (!taskLine.empty()) {
            cout << "\t\t  " << taskLine << endl;
            hasTasks = true;
        }
    }
    taskFile.close();
    remove("temp_tasks.txt");
    if (!hasTasks) {
        cout << "\t\t  (No boot tasks found)" << endl;
    }
    cout << endl;

    cout << "DEFAULT VALUES WILL BE RESTORED:" << endl;
    cout << "--------------------------------" << endl;
    showRegistryKeyValues("HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon", "Shell");
    showRegistryKeyValues("HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon", "Userinit");

    cout << "==================================================================" << endl << endl;



    cout << endl << "Press any key to continue...";
    _getch();
}

void checkShellUserinit() {
    system("cls");
    cout << "========================================" << endl;
    cout << "   Shell/Userinit Check" << endl;
    cout << "========================================" << endl << endl;

    cout << "Checking Shell and Userinit Registry Values..." << endl << endl;

//    cout << "Shell value (should be explorer.exe):" << endl;
//    cout << "--------------------------------------" << endl;
    system("reg query \"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\" /v Shell 2>nul");

    cout << endl;
//    cout << "Userinit value (should be userinit.exe,):" << endl;
//    cout << "------------------------------------------" << endl;
    system("reg query \"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\" /v Userinit 2>nul");

    cout << endl;
//    cout << "ShellExecuteHooks (potential malware location):" << endl;
//    cout << "-----------------------------------------------" << endl;
    system("reg query \"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ShellExecuteHooks\" 2>nul");

    cout << endl;
//    cout << "Additional Winlogon values:" << endl;
//    cout << "---------------------------" << endl;
    system("reg query \"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\" /v Shell 2>nul");
    system("reg query \"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\" /v Userinit 2>nul");
    system("reg query \"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\" /v Taskman 2>nul");

    cout << endl;
    cout << "------------------------------------------------------------------------------------------------------------------------" << endl;
    cout << "Commands:" << endl;
    cout << "restore_shell        - Restore Shell to explorer.exe" << endl;
    cout << "restore_userinit     - Restore Userinit to userinit.exe," << endl;
    cout << "check_malware        - Check for suspicious values" << endl;
    cout << "q (back)" << endl;
    cout << "Enter command: ";

    string command;
    getline(cin, command);

    if (command == "q" || command == "Q") {
        return;
    }
    else if (command == "restore_shell") {
        cout << "Restoring Shell to default value..." << endl;
        system(R"(reg add "HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon" /v Shell /t REG_SZ /d explorer.exe /f)");
        cout << "Shell restored to explorer.exe" << endl;
    }
    else if (command == "restore_userinit") {
        cout << "Restoring Userinit to default value..." << endl;
        system(R"(reg add \"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\" /v Userinit /t REG_SZ /d userinit.exe, /f)");
        cout << "Userinit restored to userinit.exe," << endl;
    }
    else if (command == "check_malware") {
        cout << "Checking for suspicious values..." << endl;
        cout << "Looking for non-standard Shell values:" << endl;
        system(R"(reg query "HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon" /v Shell 2>nul | findstr /v explorer.exe)");
        cout << endl << "Looking for non-standard Userinit values:" << endl;
        system(R"(reg query "HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon" /v Userinit 2>nul | findstr /v userinit.exe)");
    }
    else if (!command.empty()) {
        cout << "Unknown command. Available commands:" << endl;
        cout << "restore_shell        - Restore Shell to explorer.exe" << endl;
        cout << "restore_userinit     - Restore Userinit to userinit.exe," << endl;
        cout << "check_malware        - Check for suspicious values" << endl;
        cout << "q (back)" << endl;
    }

    cout << endl << "Press any key to continue...";
    _getch();
}

string getCurrentTimestamp() {
    auto now = chrono::system_clock::now();
    auto time_t = chrono::system_clock::to_time_t(now);
    auto tm = *localtime(&time_t);

    char buffer[100];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm);
    return string(buffer);
}

void writeLog(const string& message) {
    return;
//    ofstream logFile("startup_restore.log", ios::app);
//    if (logFile.is_open()) {
//        logFile << "[" << getCurrentTimestamp() << "] " << message << endl;
//        logFile.close();
//    }
}

void restoreStartupSettings() {
    system("cls");
    cout << "========================================" << endl;
    cout << "   Restore to Original" << endl;
    cout << "========================================" << endl << endl;

    cout << "This will restore startup settings to their original state." << endl;
    cout << "WARNING: This may affect system startup behavior!" << endl << endl;

    // Показать конкретные записи реестра которые будут удалены
    cout << "The following specific registry entries and tasks will be DELETED:" << endl;
    cout << "==================================================================" << endl << endl;

    cout << "REGISTRY ENTRIES:" << endl;
    cout << "-----------------" << endl;

    // Функция для вывода текущих значений реестра
    auto showRegistryValues = [](const string& regPath, const string& description = "") {
        cout << "• " << regPath;
        if (!description.empty()) {
            cout << " (" << description << ")";
        }
        cout << endl;

        // Создаем временный файл для вывода reg query
        string tempFile = "temp_reg_output.txt";
        string command = "reg query \"" + regPath + "\" 2>nul > " + tempFile;
        system(command.c_str());

        // Читаем и выводим содержимое
        ifstream file(tempFile);
        string line;
        bool hasContent = false;

        while (getline(file, line)) {
            if (!line.empty() && line.find(regPath) == string::npos) {
                if (!hasContent) {
                    cout << "\t\tCurrent values:" << endl;
                    hasContent = true;
                }
                cout << "\t\t  " << line << endl;
            }
        }
        file.close();

        // Удаляем временный файл
        remove(tempFile.c_str());

        if (!hasContent) {
            cout << "\t\t(No values found)" << endl;
        }
        cout << endl;
    };

    // Функция для вывода значений конкретных ключей
    auto showRegistryKeyValues = [](const string& regPath, const string& valueName = "") {
        cout << "• " << regPath;
        if (!valueName.empty()) {
            cout << "\\" << valueName;
        }
        cout << endl;

        string tempFile = "temp_reg_output.txt";
        string command;
        if (valueName.empty()) {
            command = "reg query \"" + regPath + "\" 2>nul > " + tempFile;
        } else {
            command = "reg query \"" + regPath + "\" /v \"" + valueName + "\" 2>nul > " + tempFile;
        }
        system(command.c_str());

        ifstream file(tempFile);
        string line;
        bool hasContent = false;

        while (getline(file, line)) {
            if (!line.empty() && line.find(regPath) == string::npos) {
                if (!hasContent) {
                    cout << "\t\tCurrent value: ";
                    hasContent = true;
                }
                cout << line << endl;
            }
        }
        file.close();

        remove(tempFile.c_str());

        if (!hasContent) {
            cout << "\t\t(Value not set)" << endl;
        }
        cout << endl;
    };

    // Вывод всех записей с текущими значениями
    showRegistryValues("HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", "User startup programs");
    showRegistryValues("HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", "Machine startup programs");
    showRegistryValues("HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce", "User run once programs");
    showRegistryValues("HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce", "Machine run once programs");
    showRegistryValues("HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\RunServices", "User services");
    showRegistryValues("HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\RunServices", "Machine services");
    showRegistryValues("HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\RunServicesOnce", "User run once services");
    showRegistryValues("HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\RunServicesOnce", "Machine run once services");
    showRegistryValues("HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ShellExecuteHooks", "Shell execute hooks");
    showRegistryValues("HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\SharedTaskScheduler", "Shared tasks");
    showRegistryValues("HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Browser Helper Objects", "Browser helpers");
    showRegistryValues("HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\Run", "Machine policies");
    showRegistryValues("HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\Run", "User policies");
    showRegistryValues("HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\Run", "Machine approved startup");
    showRegistryValues("HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\Run", "User approved startup");
    showRegistryValues("HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\Run32", "Machine 32-bit approved");
    showRegistryValues("HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\Run32", "User 32-bit approved");
    showRegistryValues("HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\StartupFolder", "Machine startup folder");
    showRegistryValues("HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\StartupFolder", "User startup folder");
    showRegistryValues("HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\StartupFolderCommon", "Machine common startup");
    showRegistryValues("HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\StartupFolderCommon", "User common startup");

    // ВАЖНО: Не показываем и не удаляем HKLM\SYSTEM\CurrentControlSet\Services - это критически важно!
    cout << "• HKLM\\SYSTEM\\CurrentControlSet\\Services (System services)" << endl;
    cout << "\t\t(SKIPPED - Contains critical system drivers and services)" << endl << endl;

    cout << "WINLOGON VALUES:" << endl;
    cout << "----------------" << endl;
    showRegistryKeyValues("HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon", "Taskman");
    showRegistryKeyValues("HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon", "AppSetup");
    showRegistryKeyValues("HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon", "System");

    cout << "TASK SCHEDULER TASKS:" << endl;
    cout << "---------------------" << endl;
    cout << "• All tasks containing 'startup' in name" << endl;
    // Показ задач планировщика
    system("schtasks /query /fo list | findstr /i \"startup\" > temp_tasks.txt 2>nul");
    ifstream taskFile("temp_tasks.txt");
    string taskLine;
    bool hasTasks = false;
    cout << "\t\tCurrent tasks:" << endl;
    while (getline(taskFile, taskLine)) {
        if (!taskLine.empty()) {
            cout << "\t\t  " << taskLine << endl;
            hasTasks = true;
        }
    }
    taskFile.close();
    remove("temp_tasks.txt");
    if (!hasTasks) {
        cout << "\t\t  (No startup tasks found)" << endl;
    }
    cout << endl;

    cout << "• All tasks containing 'logon' in name" << endl;
    system("schtasks /query /fo list | findstr /i \"logon\" > temp_tasks.txt 2>nul");
    taskFile.open("temp_tasks.txt");
    hasTasks = false;
    cout << "\t\tCurrent tasks:" << endl;
    while (getline(taskFile, taskLine)) {
        if (!taskLine.empty()) {
            cout << "\t\t  " << taskLine << endl;
            hasTasks = true;
        }
    }
    taskFile.close();
    remove("temp_tasks.txt");
    if (!hasTasks) {
        cout << "\t\t  (No logon tasks found)" << endl;
    }
    cout << endl;

    cout << "• All tasks containing 'boot' in name" << endl;
    cout << "\t\t(SKIPPED - Contains critical system boot tasks)" << endl << endl;

    cout << "DEFAULT VALUES WILL BE RESTORED:" << endl;
    cout << "--------------------------------" << endl;
    showRegistryKeyValues("HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon", "Shell");
    showRegistryKeyValues("HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon", "Userinit");

    cout << "==================================================================" << endl << endl;

    // Запрос подтверждения
    cout << "ARE YOU SURE YOU WANT TO DELETE ALL THESE ENTRIES? (y/n): ";
    char choice;
    cin >> choice;

    if (choice != 'y' && choice != 'Y') {
        cout << "Operation cancelled by user." << endl;
        writeLog("Startup restore operation cancelled by user");
        cout << endl << "Press any key to continue...";
        _getch();
        return;
    }

    cout << endl << "Starting restoration process..." << endl << endl;

    // Restore default registry values
    cout << "[1/7] Restoring default registry values..." << endl;
    cout << "  Setting: HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\Shell = explorer.exe" << endl;
    system("reg add \"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\" /v Shell /t REG_SZ /d explorer.exe /f");
    cout << "  Setting: HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\Userinit = userinit.exe," << endl;
    system("reg add \"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\" /v Userinit /t REG_SZ /d \"C:\\Windows\\system32\\userinit.exe,\" /f");
    writeLog("Restored default Shell and Userinit values");
    cout << "  ✓ Default registry values restored" << endl << endl;

    // Clear ALL registry startup entries
    cout << "[2/7] Clearing ALL registry startup entries..." << endl;
    cout << "  Deleting: HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Run" << endl;
    system("reg delete \"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Run\" /f");
    cout << "  Deleting: HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Run" << endl;
    system("reg delete \"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Run\" /f");
    cout << "  Deleting: HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce" << endl;
    system("reg delete \"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce\" /f");
    cout << "  Deleting: HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce" << endl;
    system("reg delete \"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce\" /f");
    cout << "  Deleting: HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\RunServices" << endl;
    system("reg delete \"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\RunServices\" /f");
    cout << "  Deleting: HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\RunServices" << endl;
    system("reg delete \"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\RunServices\" /f");
    cout << "  Deleting: HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\RunServicesOnce" << endl;
    system("reg delete \"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\RunServicesOnce\" /f");
    cout << "  Deleting: HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\RunServicesOnce" << endl;
    system("reg delete \"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\RunServicesOnce\" /f");
    writeLog("Cleared ALL Run registry entries (Run, RunOnce, RunServices, RunServicesOnce)");
    cout << "  ✓ Registry startup entries cleared" << endl << endl;

    // Clear ShellExecuteHooks and other startup locations
    cout << "[3/7] Clearing ShellExecuteHooks and Browser Helper Objects..." << endl;
    cout << "  Deleting: HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ShellExecuteHooks" << endl;
    system("reg delete \"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ShellExecuteHooks\" /f");
    cout << "  Deleting: HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\SharedTaskScheduler" << endl;
    system("reg delete \"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\SharedTaskScheduler\" /f");
    cout << "  Deleting: HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Browser Helper Objects" << endl;
    system("reg delete \"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Browser Helper Objects\" /f");
    writeLog("Cleared ShellExecuteHooks, SharedTaskScheduler, and Browser Helper Objects");
    cout << "  ✓ ShellExecuteHooks and BHO cleared" << endl << endl;

    // Clear additional startup locations
    cout << "[4/7] Clearing additional startup locations..." << endl;
    cout << "  Deleting: HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\Run" << endl;
    system("reg delete \"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\Run\" /f");
    cout << "  Deleting: HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\Run" << endl;
    system("reg delete \"HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\Run\" /f");
    cout << "  Deleting: HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\Run" << endl;
    system("reg delete \"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\Run\" /f");
    cout << "  Deleting: HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\Run" << endl;
    system("reg delete \"HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\Run\" /f");
    cout << "  Deleting: HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\Run32" << endl;
    system("reg delete \"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\Run32\" /f");
    cout << "  Deleting: HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\Run32" << endl;
    system("reg delete \"HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\Run32\" /f");
    writeLog("Cleared additional startup locations (Policies, StartupApproved)");
    cout << "  ✓ Additional startup locations cleared" << endl << endl;

    // Clear Task Scheduler startup tasks (без boot задач)
    cout << "[5/7] Clearing Task Scheduler startup tasks..." << endl;
    cout << "  Deleting tasks with: *startup*" << endl;
    system("schtasks /delete /tn \"*startup*\" /f");
    cout << "  Deleting tasks with: *logon*" << endl;
    system("schtasks /delete /tn \"*logon*\" /f");
    cout << "  Skipping tasks with: *boot* (critical system tasks)" << endl;
    writeLog("Cleared Task Scheduler startup and logon tasks (boot tasks skipped)");
    cout << "  ✓ Task Scheduler startup tasks cleared" << endl << endl;

//    // Clear Winlogon additional values
//    cout << "[6/8] Clearing Winlogon additional values..." << endl;
//    cout << "  Deleting: HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\Taskman" << endl;
//    system("reg delete \"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\" /v Taskman /f");
//    cout << "  Deleting: HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\AppSetup" << endl;
//    system("reg delete \"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\" /v AppSetup /f");
//    cout << "  Deleting: HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\System" << endl;
//    system("reg delete \"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\" /v System /f");
//    writeLog("Cleared additional Winlogon values (Taskman, AppSetup, System)");
//    cout << "  ✓ Winlogon additional values cleared" << endl << endl;

    // ПРОПУСКАЕМ удаление системных служб - это критически важно!
    cout << "[6/7] Checking services startup entries..." << endl;
    cout << "  SKIPPING: HKLM\\SYSTEM\\CurrentControlSet\\Services" << endl;
    cout << "  This section contains critical system drivers and services" << endl;
    cout << "  Deleting it would make the system unbootable!" << endl;
    writeLog("Skipped services deletion - critical system section");
    cout << "  ✓ Services section preserved (system stability)" << endl << endl;

    // Clear additional startup locations
    cout << "[7/7] Clearing additional startup locations..." << endl;
    cout << "  Deleting: HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\StartupFolder" << endl;
    system("reg delete \"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\StartupFolder\" /va /f 2>nul");
    cout << "  Deleting: HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\StartupFolder" << endl;
    system("reg delete \"HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\StartupFolder\" /va /f 2>nul");
    cout << "  Deleting: HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\StartupFolderCommon" << endl;
    system("reg delete \"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\StartupFolderCommon\" /va /f 2>nul");
    cout << "  Deleting: HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\StartupFolderCommon" << endl;
    system("reg delete \"HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\StartupFolderCommon\" /va /f 2>nul");


    writeLog("Cleared StartupApproved folder entries");
    cout << "  ✓ Additional startup locations cleared" << endl << endl;

    cout << "========================================" << endl;
    cout << "ALL startup entries cleared successfully!" << endl;
    cout << "System-critical entries were preserved for stability." << endl;
    cout << "========================================" << endl;
    writeLog("Complete startup cleanup completed successfully (critical sections preserved)");

    cout << endl << "Press any key to continue...";
    _getch();
}

//void restoreStartupSettings() {
//    system("cls");
//    cout << "========================================" << endl;
//    cout << "   Restore to Original" << endl;
//    cout << "========================================" << endl << endl;
//
//    cout << "This will restore startup settings to their original state." << endl;
//    cout << "WARNING: This may affect system startup behavior!" << endl << endl;
//
//
//
//    // Restore default registry values
//    cout << "Restoring default registry values..." << endl;
//    system("reg add \"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\" /v Shell /t REG_SZ /d explorer.exe /f >nul 2>&1");
//    system("reg add \"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\" /v Userinit /t REG_SZ /d userinit.exe, /f >nul 2>&1");
//    writeLog("Restored default Shell and Userinit values");
//
//    // Clear ALL registry startup entries
//    cout << "Clearing ALL registry startup entries..." << endl;
//    system("reg delete \"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Run\" /f >nul 2>&1");
//    system("reg delete \"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Run\" /f >nul 2>&1");
//    system("reg delete \"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce\" /f >nul 2>&1");
//    system("reg delete \"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce\" /f >nul 2>&1");
//    system("reg delete \"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\RunServices\" /f >nul 2>&1");
//    system("reg delete \"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\RunServices\" /f >nul 2>&1");
//    system("reg delete \"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\RunServicesOnce\" /f >nul 2>&1");
//    system("reg delete \"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\RunServicesOnce\" /f >nul 2>&1");
//    writeLog("Cleared ALL Run registry entries (Run, RunOnce, RunServices, RunServicesOnce)");
//
//    // Clear ShellExecuteHooks and other startup locations
//    system("reg delete \"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ShellExecuteHooks\" /f >nul 2>&1");
//    system("reg delete \"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\SharedTaskScheduler\" /f >nul 2>&1");
//    system("reg delete \"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Browser Helper Objects\" /f >nul 2>&1");
//    writeLog("Cleared ShellExecuteHooks, SharedTaskScheduler, and Browser Helper Objects");
//
//    // Clear additional startup locations
//    cout << "Clearing additional startup locations..." << endl;
//    system("reg delete \"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\Run\" /f >nul 2>&1");
//    system("reg delete \"HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\Run\" /f >nul 2>&1");
//    system("reg delete \"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\Run\" /f >nul 2>&1");
//    system("reg delete \"HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\Run\" /f >nul 2>&1");
//    system("reg delete \"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\Run32\" /f >nul 2>&1");
//    system("reg delete \"HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\Run32\" /f >nul 2>&1");
//    writeLog("Cleared additional startup locations (Policies, StartupApproved)");
//
//    // Clear Task Scheduler startup tasks
//    cout << "Clearing Task Scheduler startup tasks..." << endl;
//    system("schtasks /delete /tn \"*startup*\" /f >nul 2>&1");
//    system("schtasks /delete /tn \"*logon*\" /f >nul 2>&1");
//    system("schtasks /delete /tn \"*boot*\" /f >nul 2>&1");
//    writeLog("Cleared Task Scheduler startup, logon, and boot tasks");
//
//    // Clear Winlogon additional values
//    cout << "Clearing Winlogon additional values..." << endl;
//    system("reg delete \"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\" /v Taskman /f >nul 2>&1");
//    system("reg delete \"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\" /v AppSetup /f >nul 2>&1");
//    system("reg delete \"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\" /v System /f >nul 2>&1");
//    writeLog("Cleared additional Winlogon values (Taskman, AppSetup, System)");
//
//    // Clear services startup
//    cout << "Clearing services startup entries..." << endl;
//    system("reg delete \"HKLM\\SYSTEM\\CurrentControlSet\\Services\" /f >nul 2>&1");
//    writeLog("Cleared services startup entries");
//
//    // Clear additional startup locations
//    cout << "Clearing additional startup locations..." << endl;
//    system("reg delete \"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\StartupFolder\" /f >nul 2>&1");
//    system("reg delete \"HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\StartupFolder\" /f >nul 2>&1");
//    system("reg delete \"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\StartupFolderCommon\" /f >nul 2>&1");
//    system("reg delete \"HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\StartupFolderCommon\" /f >nul 2>&1");
//    writeLog("Cleared StartupApproved folder entries");
//
//    cout << endl << "ALL startup entries cleared successfully!" << endl;
//    writeLog("Complete startup cleanup completed successfully");
//
//    cout << endl << "Press any key to continue...";
//    _getch();
//}
