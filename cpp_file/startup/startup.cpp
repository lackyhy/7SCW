//
// Created by User on 29.09.2025.
//


#include "iostream"
#include "string"

#include <conio.h>
#include <shlobj.h>
#include <sstream>
#include "windows.h"
#include "../../h_file/startup/startup.h"


using namespace std;


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

void checkStartupFolder() {
    system("cls");
    cout << "========================================" << endl;
    cout << "   Startup Folder Check" << endl;
    cout << "========================================" << endl << endl;

    cout << "Checking Startup Folders..." << endl << endl;

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

