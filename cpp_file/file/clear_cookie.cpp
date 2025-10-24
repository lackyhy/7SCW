#include <iostream>
#include <cstdio>
#include <vector>
#include <windows.h>
#include <shlobj.h>
#include <conio.h>
#include <string>
#include <iomanip>

#include "../../h_file/main.h"
#include "../../h_file/logs/logs.h"

using namespace std;

char userProfile[MAX_PATH];

bool DeleteFileWithErrors(string cookie_path) {
    printInfo("Attempting to delete: " + cookie_path);
    
    
    DWORD fileAttributes = GetFileAttributesA(cookie_path.c_str());
    if (fileAttributes == INVALID_FILE_ATTRIBUTES) {
        DWORD error = GetLastError();
        if (error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND) {
            printWarning("File not found: " + cookie_path);
        } else {
            printError("Cannot access file: " + cookie_path + " (Error: " + to_string(error) + ")");
        }
        return false;
    }
    
    // Проверка, является ли путь директорией
    if (fileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        printError("Path is a directory, not a file: " + cookie_path);
        return false;
    }
    
    // Проверка прав доступа
    if (fileAttributes & FILE_ATTRIBUTE_READONLY) {
        printWarning("File is read-only. Attempting to remove readonly attribute...");
        if (!SetFileAttributesA(cookie_path.c_str(), fileAttributes & ~FILE_ATTRIBUTE_READONLY)) {
            printError("Failed to remove readonly attribute (Error: " + to_string(GetLastError()) + ")");
            return false;
        }
    }
    
    // Попытка удаления файла
    if (remove(cookie_path.c_str()) == 0) {
        printMessage("File deleted successfully: " + cookie_path);
        return true;
    } else {
        DWORD error = GetLastError();
        string errorMsg;
        
        switch (error) {
            case ERROR_ACCESS_DENIED:
                errorMsg = "Access denied - file may be in use by another program";
                break;
            case ERROR_SHARING_VIOLATION:
                errorMsg = "File is being used by another process";
                break;
            case ERROR_FILE_NOT_FOUND:
                errorMsg = "File not found (may have been already deleted)";
                break;
            default:
                errorMsg = "Unknown error (Code: " + to_string(error) + ")";
                break;
        }
        
        printError("Failed to delete file: " + cookie_path + " - " + errorMsg);
        return false;
    }
}

void drawMenu_(const vector<string>& menuItems, int selectedIndex) {
    system("cls");

    // Заголовок с цветом
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    cout << "=== Browser Cookie Cleaner ===" << endl;
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    
    cout << "Use Up/Down arrows to navigate, Enter to select, 'q' to exit\n" << endl;

    // Отображение пунктов меню
    for (int i = 0; i < menuItems.size(); i++) {
        bool isSelected = (i == selectedIndex);
        
        if (isSelected) {
            SetConsoleTextAttribute(hConsole, BACKGROUND_BLUE | FOREGROUND_INTENSITY);
            cout << "> " << menuItems[i];
            
            // Добавляем индикатор для текущего выбора
            SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
            cout << " <";
        } else {
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            cout << "  " << menuItems[i] << "  ";
        }
        cout << endl;
    }

    // Сброс цвета
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    cout << "\n" << string(40, '=') << endl;
}

void handleFirefoxCookies(const string& basePath) {
    WIN32_FIND_DATAA findData;
    HANDLE hFind;
    int deletedCount = 0;
    int totalProfiles = 0;
    
    printInfo("Searching for Firefox profiles in: " + basePath);
    
    // Ищем все папки профилей
    string searchPath = basePath + "*";
    hFind = FindFirstFileA(searchPath.c_str(), &findData);
    
    if (hFind == INVALID_HANDLE_VALUE) {
        printError("Cannot access Firefox profiles directory: " + basePath);
        return;
    }
    
    do {
        // Проверяем, что это директория и не является "." или ".."
        if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && 
            string(findData.cFileName) != "." && 
            string(findData.cFileName) != "..") {
            
            totalProfiles++;
            string profileName = findData.cFileName;
            string profilePath = basePath + profileName + "\\";
            
            printInfo("Checking profile: " + profileName);
            
            // Список возможных файлов cookies в Firefox
            vector<string> cookieFiles = {
                "cookies.sqlite",
                "cookies.sqlite-wal",
                "cookies.sqlite-shm",
                "cookies.db",
                "cookies.bak"
            };
            
            // Пробуем удалить каждый возможный файл cookies
            for (const auto& cookieFile : cookieFiles) {
                string fullCookiePath = profilePath + cookieFile;
                
                if (GetFileAttributesA(fullCookiePath.c_str()) != INVALID_FILE_ATTRIBUTES) {
                    printInfo("Found cookie file: " + cookieFile + " in profile " + profileName);
                    if (DeleteFileWithErrors(fullCookiePath)) {
                        deletedCount++;
                    }
                }
            }
            
            // Также проверяем папку storage для дополнительных файлов cookies
            string storagePath = profilePath + "storage\\default\\";
            vector<string> storageCookieFiles = {
                "https+++localhost.lweb\\cookies.sqlite",
                "http+++localhost.lweb\\cookies.sqlite"
            };
            
            // Проверяем существование папки storage
            if (GetFileAttributesA(storagePath.c_str()) != INVALID_FILE_ATTRIBUTES) {
                for (const auto& storageFile : storageCookieFiles) {
                    string fullStoragePath = storagePath + storageFile;
                    if (GetFileAttributesA(fullStoragePath.c_str()) != INVALID_FILE_ATTRIBUTES) {
                        printInfo("Found storage cookie file: " + storageFile + " in profile " + profileName);
                        if (DeleteFileWithErrors(fullStoragePath)) {
                            deletedCount++;
                        }
                    }
                }
            }
        }
    } while (FindNextFileA(hFind, &findData));
    
    FindClose(hFind);
    
    // Итоговый отчет
    cout << endl;
    if (deletedCount > 0) {
        printMessage("Successfully deleted " + to_string(deletedCount) + " cookie files from " + to_string(totalProfiles) + " Firefox profiles");
    } else if (totalProfiles > 0) {
        printWarning("No cookie files found in " + to_string(totalProfiles) + " Firefox profiles");
    } else {
        printError("No Firefox profiles found in: " + basePath);
    }
}

void handleBrowserSelection(int browserIndex, const string& browserName) {
    string cookiePath;
    bool success = false;
    
    printInfo("Processing " + browserName + "...");

    switch (browserIndex) {
        case 0: // Google Chrome
            cookiePath = string(userProfile) + "\\AppData\\Local\\Google\\Chrome\\User Data\\Default\\Network\\Cookies";
            success = DeleteFileWithErrors(cookiePath);
            break;
            
        case 1: // Yandex
            cookiePath = string(userProfile) + "\\AppData\\Local\\Yandex\\YandexBrowser\\User Data\\Default\\Network\\Cookies";
            success = DeleteFileWithErrors(cookiePath);
            break;
            
        case 2: // FireFox
            cookiePath = string(userProfile) + "\\AppData\\Roaming\\Mozilla\\Firefox\\Profiles\\";
            handleFirefoxCookies(cookiePath);
            if (success) {
                success = true;
            } else {
                success = false;
            }
            break;
            
        case 3: // Opera
            cookiePath = string(userProfile) + "\\AppData\\Roaming\\Opera Software\\Opera Stable\\Network\\Cookies";
            success = DeleteFileWithErrors(cookiePath);
            break;
            
        case 4: // Microsoft Edge
            cookiePath = string(userProfile) + "\\AppData\\Local\\Microsoft\\Edge\\User Data\\Default\\Network\\Cookies";
            success = DeleteFileWithErrors(cookiePath);
            break;
            
        default:
            printError("Invalid browser selection");
            return;
    }

    // Итоговое сообщение
    cout << endl;
    if (success) {
        printMessage(browserName + " cookies cleaned successfully!");
    } else {
        printError("Failed to clean " + browserName + " cookies. The browser might be running - please close it and try again.");
    }
    cout << endl;
}

void main_menu_cookie() {
    // Инициализация userProfile с обработкой ошибок
    if (FAILED(SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, 0, userProfile))) {
        printError("Failed to get user profile path");
        return;
    }

    vector<string> menuItems = {
        "Google Chrome",
        "Yandex", 
        "FireFox",
        "Opera",
        "Microsoft Edge"
    };
    
    int selectedIndex = 0;
    bool running = true;
    
    while (running) {
        drawMenu_(menuItems, selectedIndex);

        int key = _getch();
        
        // Обработка Ctrl+C
        if (g_ctrlCPressed) {
            g_ctrlCPressed = FALSE;
            printInfo("Interrupted by user");
            running = false;
            continue;
        }
        
        if (key == 224) { // Специальные клавиши
            key = _getch();
            switch (key) {
                case 72: // Up arrow
                    selectedIndex = (selectedIndex - 1 + menuItems.size()) % menuItems.size();
                    break;
                case 80: // Down arrow
                    selectedIndex = (selectedIndex + 1) % menuItems.size();
                    break;
            }
        } else if (key == 13) { // Enter key
            string browserName = menuItems[selectedIndex];
            handleBrowserSelection(selectedIndex, browserName);
            
            cout << "Press any key to continue...";
            _getch();
            
        } else if (key == 'q' || key == 'Q') {
            printInfo("Exiting Browser Cookie Cleaner");
            running = false;
        } else if (key == 'h' || key == 'H') {
            system("cls");
            printInfo("Help: Use arrow keys to navigate, Enter to select, 'q' to quit");
            cout << "This tool helps clean browser cookies. Make sure browsers are closed before cleaning." << endl;
            cout << "\nPress any key to continue...";
            _getch();
        }
    }
}