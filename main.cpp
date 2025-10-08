#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <ctime>

#include <windows.h>
#include <conio.h>


#include "h_file/startup/restoreStartupSettings.h"
#include "h_file/startup/startup.h"
#include "h_file/terminal/terminal.h"
#include "h_file/file_manager/file_manager.h"
#include "h_file/startup/SHOW_ALL_STARTUP.h"
#include "h_file/security/advanced_security_menu.h"
#include "h_file/main.h"
#include "h_file/argv.h"
#include "h_file/file/clear_temp_file.h"

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
    cout << "      - Right arrow to open custom terminal" << endl;
    cout << "      - Left arrow to open Advanced Security Menu" << endl << endl;

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

    cout << "   Advanced Security Menu:" << endl;
    cout << "       - Available from main menu by pressing left arrow" << endl;
    cout << "       - File Hash Verification: Check system file integrity" << endl;
    cout << "       - Event Logs: View Windows, Application, Security, and System logs" << endl;
    cout << "       - Custom Log Search: Search through logs with filters" << endl;
    cout << "       - System Integrity Check: Comprehensive system health check" << endl;
    cout << "       - Security Statistics: System security overview" << endl;
    cout << "       - Export Security Report: Generate security reports" << endl << endl;

    cout << "   Additional Programs:" << endl;
    cout << "       - Access to various utilities such as SimpleUnlocker, Registry Workshop, ProcessHacker, etc." << endl << endl;

    cout << "       Press any key to return to the main menu..." << endl << endl;
    cout << "   Arguments:" << endl;
    cout << "       -clear_tempfile - clear temp file" << endl;
    cout << "       -clear_autorun - clear Startup" << endl;
    cout << "       -safemod - run in safe mode (works in current terminal without admin rights)" << endl;

    _getch();
}

// Function to draw menu
void drawMenu(const vector<string>& menuItems, int selectedIndex) {
    system("cls");

    cout << "Main Menu" << endl;
    cout << "Use Up and Down arrows to navigate, Enter to select, 'h' for help, 'q' to exit" << endl << endl;

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    for (int i = 0; i < menuItems.size(); i++) {
        bool isSel = (i == selectedIndex);
        if (isSel) SetConsoleTextAttribute(hConsole, BACKGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
        cout << (isSel ? ">" : "   ") << menuItems[i] << endl;
        if (isSel) SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    }

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    int consoleWidth = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    cout << "\n\n";
    cout << endl;


    for(int i = 0; i < consoleWidth; ++i) {
        cout << "-";
    }

    cout << "Version: " << VERSION << " | Created by: LCKY |GitHub: https://github.com/lackyhy/7SCW" << endl;

    for(int i = 0; i < consoleWidth; ++i) {
        cout << "-";
    }
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

        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        for (int i = 0; i < options.size(); i++) {
            bool isSel = (i == selectedIndex);
            if (isSel) SetConsoleTextAttribute(hConsole, BACKGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);

            // Set color for "Restore to original" option text
            if (options[i] == "Restore to original\n") {
                if (!isSel) SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
            }
            cout << (isSel ? ">" : "   ") << options[i] << endl;

            // Reset to default after each line
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
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

void main_menu(bool safemod, bool isAdmin) {
    BOOL safemode = safemod;

    BOOL clear_tempfile = FALSE;
    BOOL clear_autorun = FALSE;

    if ( safemod ) {
        stringstream title;
        title << "Menu System [SAFE MODE], SafeMode: " << (safemode ? "true" : "false") << ", isAdmin: " << (isAdmin ? "true" : "false");
        SetConsoleTitleA(title.str().c_str());
        // В safemode работаем в текущем терминале без создания нового окна
        vector <string> menuItems = {
                "File Manager",
                "Check Startup",
                "Users\n",
                "Clear TEMP Files",
                "System Info\n",
                "Log Viewer & Security",
                "File Hash Verification\n",
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
                    case 75: // Left arrow - Advanced Security Menu
                        showAdvancedSecurityMenu();
                        break;
                    case 77: // Right arrow - Open custom terminal
                        customTerminal();
                        break;
                    case 80: // Down arrow
                        selectedIndex = (selectedIndex + 1) % menuItems.size();
                        break;
                }
            } else if (key == 13) { // Enter key
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
                        clear_temp_file();
                        _getch();
                        break;
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
            } else if (key == 'h' || key == 'H') {
                showHelp();
            } else if (key == 'q' || key == 'Q') {
                running = false;
            }
        }
        return;

    } else {

        if (!isAdmin) {
            cout << "The program must be run as administrator!" << endl;
            cout << "Run with '-safemod' argument for limited functionality without admin rights." << endl;
            cout << "Use '-help' argument to help" << endl;
            cout << "Press any key to exit...";
            _getch();
            return;
        }
        stringstream title;
        title << (safemode ? "Menu System [SAFE MODE]" : "Menu System") << ",            isAdmin: " << (isAdmin ? "true" : "false");
        SetConsoleTitleA(title.str().c_str());
        hideCursor();


        // Register console control handler
        if (!SetConsoleCtrlHandler(CtrlHandler, TRUE)) {
            // Handle error if registration fails
            cout << "Error: Failed to register Ctrl+C handler!" << endl;
            // Decide whether to exit or continue
        }

        vector <string> menuItems = {
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
                    case 75: // Left arrow - Advanced Security Menu
                        showAdvancedSecurityMenu();
                        break;
                    case 77: // Right arrow - Open custom terminal
                        customTerminal();
                        break;
                    case 80: // Down arrow
                        selectedIndex = (selectedIndex + 1) % menuItems.size();
                        break;
                }
            } else if (key == 13) { // Enter key
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
                        clear_temp_file();
                        _getch();
                        break;
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
            } else if (key == 'h' || key == 'H') {
                showHelp();
            } else if (key == 'q' || key == 'Q') {
                running = false;
            }
        }

        showCursor();
        return;
    }
};


int main(int argc, char *argv[]) {
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID AdministratorsGroup;
    if (AllocateAndInitializeSid(&NtAuthority, 2,
                                 SECURITY_BUILTIN_DOMAIN_RID,
                                 DOMAIN_ALIAS_RID_ADMINS,
                                 0, 0, 0, 0, 0, 0,
                                 &AdministratorsGroup)) {
        CheckTokenMembership(NULL, AdministratorsGroup, &isAdmin);
        FreeSid(AdministratorsGroup);
    }

    process_command_line_args(argc, argv);

    process_argc();


    // start menu
    main_menu(safemode, isAdmin);
    return 0;
}
