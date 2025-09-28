#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <ctime>

#include <windows.h>
#include <conio.h>
#include <shlobj.h>

#include "h_file/startup/restoreStartupSettings.h"
#include "h_file/startup/startup.h"
#include "h_file/terminal/terminal.h"
#include "h_file/file_manager/file_manager.h"
#include "h_file/startup/SHOW_ALL_STARTUP.h"
#include "h_file/main.h"

using namespace std;

volatile BOOL g_ctrlCPressed = FALSE;

BOOL WINAPI CtrlHandler(DWORD fdwCtrlType) {
    switch (fdwCtrlType) {
        case CTRL_C_EVENT:
            g_ctrlCPressed = TRUE;
            return TRUE;
        default:
            return FALSE;
    }
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

void showStartupLocationsMenu();


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

    cout << "Version: 8.3.5 | Created by: LCKY |GitHub: https://github.com/lackyhy/7SCW" << endl;

    for(int i = 0; i < consoleWidth; ++i) {
        cout << "-";
    }
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
                    file_manger();
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