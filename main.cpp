#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <ctime>

#include <windows.h>
#include <conio.h>

#include "include/h_file/startup/restoreStartupSettings.h"
#include "include/h_file/startup/startup.h"
#include "include/h_file/terminal/terminal.h"
#include "include/h_file/file_manager/file_manager.h"
#include "include/h_file/startup/SHOW_ALL_STARTUP.h"
#include "include/h_file/security/advanced_security_menu.h"
#include "include/h_file/main.h"
#include "include/h_file/argv.h"
#include "include/h_file/file/clear_temp_file.h"
#include "include/h_file/terminal/terminal_commands.h"
#include "include/h_file/system_info/system_info.h"
#include "include/Logger.h"
#include "include/h_file/dnsSSL/dnsSSL.h"
#include "include/h_file/show_web/show_web.h"
#include "include/h_file/users/users_manager.h"
#include "include/h_file/pstools/pstools_manager.h"

using namespace std;

volatile BOOL g_ctrlCPressed = FALSE;

BOOL WINAPI CtrlHandler(DWORD fdwCtrlType)
{
    switch (fdwCtrlType)
    {
    case CTRL_C_EVENT:
        g_ctrlCPressed = TRUE;
        return TRUE;
    default:
        return FALSE;
    }
}

void showStartupLocationsMenu();

void hideCursor()
{
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
    cursorInfo.bVisible = false;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
}

void showCursor()
{
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
    cursorInfo.bVisible = true;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
}

void showHelp()
{
    system("cls");
    cout << "   File Manager and System Tools Help" << endl
         << endl;
    cout << "   Main Menu:" << endl;
    cout << "      - Use Up/Down arrows to navigate" << endl;
    cout << "      - Enter to select an option" << endl;
    cout << "      - Right arrow to open custom terminal" << endl;
    cout << "      - Left arrow to open Advanced Security Menu" << endl
         << endl;

    cout << "   File Manager:" << endl;
    cout << "      - Navigate directories using Up/Down arrows" << endl;
    cout << "      - Enter to open folders or select files" << endl;
    cout << "      - 'P' / 'p' to quick preview text files" << endl;
    cout << "      - 'O' / 'o' for Additional Operations (Copy, Move, Delete, Rename)" << endl;
    cout << "      - 'S' / 's' to search for files on the computer" << endl;
    cout << "      - Right arrow on a file/folder for detailed info & MD5/SHA256 checksums" << endl;
    cout << "      - Left arrow on a file/folder for management options & attribute editing" << endl;
    cout << "      - 'q' to return to previous menu" << endl;
    cout << "      - 'b' to go back in directory history" << endl
         << endl;

    cout << "   System Tools:" << endl;
    cout << "       - Check Startup Locations: Interactive examination of startup locations with management commands" << endl;
    cout << "         * Startup Folders: View and open startup folders" << endl;
    cout << "         * Registry: View, add, and delete registry startup entries" << endl;
    cout << "         * Task Scheduler: View and manage scheduled startup tasks" << endl;
    cout << "         * Shell/Userinit: Check and restore critical system values" << endl;
    cout << "       - Clear TEMP Files: Remove temporary files to free up disk space" << endl;
    cout << "       - System Info: Detailed CPU, Memory (RAM load bar), GPU/Display, Storage capacity, Uptime & Adapters" << endl;
    cout << "       - Users Manager: Full User Account Manager (Create, Password Reset, Enable/Disable, Role Grant, Delete)" << endl
         << endl;

    cout << "   Key Bindings:" << endl;
    cout << "       - Up Arrow: Move selection up" << endl;
    cout << "       - Down Arrow: Move selection down" << endl;
    cout << "       - Enter: Select/Confirm" << endl;
    cout << "       - Right Arrow: Show detailed info & checksums (in File Manager)" << endl;
    cout << "       - Left Arrow: Management menu & attribute editing (in File Manager)" << endl;
    cout << "       - 'P' / 'p': Quick Text File Preview" << endl;
    cout << "       - 'O' / 'o': Additional Operations (Copy, Move, Delete, Rename)" << endl;
    cout << "       - '4': Refresh list of files/drives" << endl;
    cout << "       - 'q': Quit/Return to previous menu" << endl;
    cout << "       - 'b': Go back (in File Manager)" << endl;
    cout << "       - 'i': Show web" << endl;
    cout << "       - 'h': Show this help menu" << endl
         << endl;

    cout << "   Custom Terminal:" << endl;
    cout << "       - Available from main menu by pressing right arrow" << endl;
    cout << "       - Type 'exit' or 'quit' to return to main menu" << endl;
    cout << "       - Supports bash-like commands and Windows commands" << endl;
    cout << "       - Type 'help' in terminal for available commands" << endl
         << endl;

    cout << "   Advanced Security Menu:" << endl;
    cout << "       - Available from main menu by pressing left arrow" << endl;
    cout << "       - File Hash Verification: Check system file integrity" << endl;
    cout << "       - Event Logs: View Windows, Application, Security, and System logs" << endl;
    cout << "       - Custom Log Search: Search through logs with filters" << endl;
    cout << "       - System Integrity Check: Comprehensive system health check" << endl;
    cout << "       - Security Statistics: System security overview" << endl;
    cout << "       - Export Security Report: Generate security reports" << endl
         << endl;

    cout << "   Run as SYSTEM (PSTools):" << endl;
    cout << "       - Download PSTools (Sysinternals) directly into application directory" << endl;
    cout << "       - Launch CMD, PowerShell, Task Manager, Regedit or custom tools as NT AUTHORITY\\SYSTEM" << endl
         << endl;

    cout << "   Additional Programs:" << endl;
    cout << "       - Access to various utilities such as SimpleUnlocker, Registry Workshop, ProcessHacker, etc." << endl
         << endl;

    cout << "       Press any key to return to the main menu..." << endl
         << endl;
    cout << "   Arguments:" << endl;
    cout << "       -safemode / -safemod   - Run app in safe mode (limited rights)" << endl;
    cout << "       -clear_tempfile        - Clear system temporary files" << endl;
    cout << "       -clear_autorun         - Restore startup registry and folders" << endl;
    cout << "       --systemInfo           - Print detailed system information" << endl;
    cout << "       --terminal             - Launch interactive custom terminal" << endl;
    cout << "       --clear_logs           - Clear log history and delete logs.txt" << endl;
    cout << "       --version / -v         - Display current program version" << endl;
    cout << "       --help / -h            - Display command line help" << endl;
    cout << "       --logs                 - Enable logging to logs.txt" << endl;
    cout << "       -logs_console          - Open separate console window for logs" << endl;

    _getch();
}

void drawMenu(const vector<string> &menuItems, int selectedIndex)
{
    system("cls");

    cout << "Main Menu" << endl;
    cout << "Use Up and Down arrows to navigate, Enter to select, 'h' for help, 'q' to exit" << endl
         << endl;

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    for (int i = 0; i < menuItems.size(); i++)
    {
        bool isSel = (i == selectedIndex);
        if (isSel)
            SetConsoleTextAttribute(hConsole, BACKGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
        cout << (isSel ? ">" : "   ") << menuItems[i] << endl;
        if (isSel)
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    }

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    int consoleWidth = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    cout << "\n\n";
    cout << endl;

    for (int i = 0; i < consoleWidth; ++i)
    {
        cout << "-";
    }

    // Check for updates and display indicator
    string versionDisplay = "Version: " + string(VERSION);
    if (isUpdateAvailable())
    {
        versionDisplay += " (Update Available!)";
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    }
    cout << versionDisplay << "" << endl;
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

    for (int i = 0; i < consoleWidth; ++i)
    {
        cout << "-";
    }
}

void showStartupLocationsMenu()
{
    bool isSys = isRunningAsSystem();
    vector<string> options;
    if (isSys) {
        options = {
            "Startup Folder",
            "Task Scheduler",
            "ALL PC Startups (SYSTEM)",
            "Registry",
            "Shell/Userinit\n",
            "Restore to original\n",
            "Back to Main Menu"
        };
    } else {
        options = {
            "Startup Folder",
            "Task Scheduler",
            "ALL PC Startups",
            "Registry",
            "Shell/Userinit\n",
            "Restore to original\n",
            "Back to Main Menu"
        };
    }

    int selectedIndex = 0;
    bool running = true;

    while (running)
    {
        system("cls");
        cout << "Check Startup Locations" << (isSys ? " [NT AUTHORITY\\SYSTEM]" : "") << endl;
        cout << "Use Up and Down arrows to navigate, Enter to select, 'q' to quit" << endl
             << endl;

        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        for (int i = 0; i < options.size(); i++)
        {
            bool isSel = (i == selectedIndex);
            if (isSel)
                SetConsoleTextAttribute(hConsole, BACKGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);

            if (options[i].find("ALL PC Startups") != string::npos)
            {
                if (!isSel)
                    SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
            }

            // Set color for "Restore to original" option text
            if (options[i] == "Restore to original\n")
            {
                if (!isSel)
                    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
            }
            cout << (isSel ? ">" : "   ") << options[i] << endl;

            // Reset to default after each line
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        }

        int key = _getch();
        if (g_ctrlCPressed)
        {
            g_ctrlCPressed = FALSE;
            running = false;
            continue;
        }
        if (key == 224)
        {
            key = _getch();
            switch (key)
            {
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
        else if (key == 13)
        { // Enter
            switch (selectedIndex)
            {
            case 0: // Startup Folder
                checkStartupFolder();
                break;
            case 1: // Task Scheduler
                checkTaskScheduler();
                break;
            case 2: // ALL PC Startups (SYSTEM)
                SHOW_ALL_STARTUP();
                break;
            case 3: // Registry
                checkRegistryStartup();
                break;
            case 4: // Shell/Userinit
                checkShellUserinit();
                break;
            case 5: // Restore to original
                restoreStartupSettings();
                break;
            case 6: // Back to Main Menu
                running = false;
                break;
            }
        }
        else if (key == 'q' || key == 'Q')
        {
            running = false;
        }
    }
}

void main_menu(bool safemod, bool isAdmin)
{
    Logger::info("Starting main menu - SafeMode: " + string(safemod ? "true" : "false") + ", Admin: " + string(isAdmin ? "true" : "false"));

    if (safemod)
    {
        stringstream title;
        title << "Menu System [SAFE MODE], SafeMode: " << (safemode ? "true" : "false") << ", isAdmin: " << (isAdmin ? "true" : "false");
        SetConsoleTitleA(title.str().c_str());
        Logger::info("Running in Safe Mode");
        vector<string> menuItems = {
            "File Manager",
            "Check Startup",
            "Users\n",
            "Clear TEMP Files",
            "Clear DNS and SSL",
            "System Info\n",
            "CMD",
            "POWERSHELL\n",
            "Run as SYSTEM (PSTools)\n",
            "Help",
            "Exit"};

        int selectedIndex = 0;
        bool running = true;

        while (running)
        {
            drawMenu(menuItems, selectedIndex);

            int key = _getch();
            if (g_ctrlCPressed)
            {                           // Check for Ctrl+C
                g_ctrlCPressed = FALSE; // Reset the flag
                running = false;        // Exit the main menu loop
                continue;               // Skip the rest of the loop iteration
            }
            if (key == 224)
            { // Arrow key pressed
                key = _getch();
                switch (key)
                {
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
            }
            else if (key == 13)
            { // Enter key
                switch (selectedIndex)
                {
                case 0: // File Manager
                    Logger::info("Opening File Manager");
                    file_manger();
                    break;
                case 1: // Check Startup
                    Logger::info("Opening Startup Locations Menu");
                    showStartupLocationsMenu();
                    break;
                case 2: // Users
                    Logger::info("Opening Users Manager");
                    showUsersManagerMenu();
                    break;
                case 3: // Clear Temp File
                    Logger::info("Clearing temporary files");
                    clear_temp_file();
                    _getch();
                    break;
                case 4: // Clear DNS and SSL
                    Logger::info("Clearing DNS and SSL");
                    clear_dns_ssl_lite_mode();
                    break;
                case 5: // System Info
                    Logger::info("Displaying system information");
                    system("cls");
                    print_SystemInfo();
                    break;
                case 6: // CMD
                    Logger::info("Opening CMD");
                    system("cls");
                    system("cmd");
                    _getch();
                    break;
                case 7: // POWERSHELL
                    Logger::info("Opening PowerShell");
                    system("cls");
                    system("powershell");
                    _getch();
                    break;
                case 8: // Run as SYSTEM (PSTools)
                    Logger::info("Opening Run as SYSTEM Menu");
                    showPSToolsMenu();
                    break;
                case 9: // Help
                    Logger::info("Showing help menu");
                    showHelp();
                    break;
                case 10: // Exit
                    Logger::info("Exiting application");
                    running = false;
                    break;
                }
            }
            else if (key == 'h' || key == 'H')
            {
                showHelp();
            }
            else if (key == 'i' || key == 'I')
            {
                show_web_all();
            }
            else if (key == 'q' || key == 'Q')
            {
                running = false;
            }
        }
        return;
    }
    else
    {

        if (!isAdmin)
        {
            Logger::warning("Program must be run as administrator");
            cout << "The program must be run as administrator!" << endl;
            cout << "Run with '-safemod' argument for limited functionality without admin rights." << endl;
            cout << "Use '-help' argument to help" << endl;
            cout << "Press any key to exit...";
            _getch();
            return;
        }
        Logger::info("Running with administrator privileges");
        stringstream title;
        if (isRunningAsSystem()) {
            title << "Menu System [NT AUTHORITY\\SYSTEM], isAdmin: true";
        } else {
            title << (safemode ? "Menu System [SAFE MODE]" : "Menu System") << ",            isAdmin: " << (isAdmin ? "true" : "false");
        }
        SetConsoleTitleA(title.str().c_str());
        hideCursor();

        // Register console control handler
        if (!SetConsoleCtrlHandler(CtrlHandler, TRUE))
        {
            // Handle error if registration fails
            cout << "Error: Failed to register Ctrl+C handler!" << endl;
            // Decide whether to exit or continue
        }

        vector<string> menuItems = {
            "File Manager",
            "Check Startup",
            "Users\n",
            "Clear TEMP Files",
            "Clear DNS and SSL",
            "System Info\n",
            "CMD",
            "POWERSHELL\n",
            "Run as SYSTEM (PSTools)\n",
            "Help",
            "Exit"
        };

        int selectedIndex = 0;
        bool running = true;

        while (running)
        {
            drawMenu(menuItems, selectedIndex);

            int key = _getch();
            if (g_ctrlCPressed)
            {                           // Check for Ctrl+C
                g_ctrlCPressed = FALSE; // Reset the flag
                running = false;        // Exit the main menu loop
                continue;               // Skip the rest of the loop iteration
            }
            if (key == 224)
            { // Arrow key pressed
                key = _getch();
                switch (key)
                {
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
            }
            else if (key == 13)
            { // Enter key
                switch (selectedIndex)
                {
                case 0: // File Manager
                    Logger::info("Opening File Manager");
                    file_manger();
                    break;
                case 1: // Check Startup
                    Logger::info("Opening Startup Locations Menu");
                    showStartupLocationsMenu();
                    break;
                case 2: // Users
                    Logger::info("Opening Users Manager");
                    showUsersManagerMenu();
                    break;
                case 3: // Clear Temp File
                    Logger::info("Clearing temporary files");
                    clear_temp_file();
                    _getch();
                    break;
                case 4: // Clear DNS and SSL
                    Logger::info("Clearing DNS and SSL");
                    clear_dns_ssl_lite_mode();
                    _getch();
                    break;
                case 5: // System Info
                    Logger::info("Displaying system information");
                    system("cls");
                    print_SystemInfo();
                    _getch();
                    break;
                case 6: // CMD
                    Logger::info("Opening CMD");
                    system("cls");
                    system("cmd");
                    _getch();
                    break;
                case 7: // POWERSHELL
                    Logger::info("Opening PowerShell");
                    system("cls");
                    system("powershell");
                    _getch();
                    break;
                case 8: // Run as SYSTEM (PSTools)
                    Logger::info("Opening Run as SYSTEM Menu");
                    showPSToolsMenu();
                    break;
                case 9: // Help
                    Logger::info("Showing help menu");
                    showHelp();
                    break;
                case 10: // Exit
                    Logger::info("Exiting application");
                    running = false;
                    break;
                }
            }
            else if (key == 'h' || key == 'H')
            {
                showHelp();
            }
            else if (key == 'i' || key == 'I')
            {
                show_web_all();
            }
            else if (key == 'q' || key == 'Q')
            {
                running = false;
            }
        }

        showCursor();
        return;
    }
}

int main(int argc, char *argv[])
{
    Logger::info("Launch SID and request administrator rights");
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID AdministratorsGroup;
    if (AllocateAndInitializeSid(&NtAuthority, 2,
                                 SECURITY_BUILTIN_DOMAIN_RID,
                                 DOMAIN_ALIAS_RID_ADMINS,
                                 0, 0, 0, 0, 0, 0,
                                 &AdministratorsGroup))
    {
        CheckTokenMembership(NULL, AdministratorsGroup, &isAdmin);
        FreeSid(AdministratorsGroup);
    }

    process_command_line_args(argc, argv);
    process_argc();

    // Initialize logger if --logs or -logs_console flag is provided
    Logger::initialize(logs_enabled, logs_console);

    // Test logs if console logging is enabled
    if (logs_console)
    {
        Logger::info("Console logging test - this should appear in separate console");
        Logger::success("Console logging is working!");
    }

    // start menu
    Logger::functions_log(LOG_INFO, "main", "starting");
    Logger::info("starting MAIN MENU");
    main_menu(safemode, isAdmin);
    Logger::success("success start main menu");
    // Close log console if it was opened
    if (logs_console)
    {
        Logger::closeLogConsole();
    }

    return 0;
}