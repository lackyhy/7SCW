#include "../../h_file/pstools/pstools_manager.h"
#include "../../Logger.h"

#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <conio.h>
#include <sstream>

using namespace std;

static void createFolderRecursive(const string& path) {
    string current;
    for (char c : path) {
        current += c;
        if (c == '\\' || c == '/') {
            CreateDirectoryA(current.c_str(), NULL);
        }
    }
    CreateDirectoryA(path.c_str(), NULL);
}

static bool setPsExecEulaAccepted() {
    HKEY hKey;
    LONG result = RegCreateKeyExA(
        HKEY_CURRENT_USER,
        "Software\\Sysinternals\\PsExec",
        0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &hKey, NULL
    );
    if (result == ERROR_SUCCESS) {
        DWORD dwValue = 1;
        RegSetValueExA(hKey, "EulaAccepted", 0, REG_DWORD, (const BYTE*)&dwValue, sizeof(dwValue));
        RegCloseKey(hKey);
        return true;
    }
    return false;
}

bool isRunningAsSystem() {
    PSID systemSid = NULL;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
    if (AllocateAndInitializeSid(&ntAuthority, 1, SECURITY_LOCAL_SYSTEM_RID, 0, 0, 0, 0, 0, 0, 0, &systemSid)) {
        BOOL isSystem = FALSE;
        CheckTokenMembership(NULL, systemSid, &isSystem);
        FreeSid(systemSid);
        return isSystem == TRUE;
    }
    return false;
}

string getPSToolsDirectory() {
    char buffer[MAX_PATH];
    DWORD len = GetModuleFileNameA(NULL, buffer, MAX_PATH);
    if (len > 0) {
        string exePath(buffer);
        size_t lastSlash = exePath.find_last_of("\\/");
        if (lastSlash != string::npos) {
            return exePath.substr(0, lastSlash) + "\\PSTools";
        }
    }
    return ".\\PSTools";
}

string findPsExecPath() {
    string targetDir = getPSToolsDirectory();
    vector<string> candidates = {
        targetDir + "\\PsExec64.exe",
        targetDir + "\\PsExec.exe",
        ".\\PSTools\\PsExec64.exe",
        ".\\PSTools\\PsExec.exe",
        "C:\\Tools\\PSTools\\PsExec64.exe",
        "C:\\Tools\\PSTools\\PsExec.exe"
    };

    for (const auto& path : candidates) {
        if (GetFileAttributesA(path.c_str()) != INVALID_FILE_ATTRIBUTES) {
            return path;
        }
    }
    return "";
}

bool isPSToolsInstalled() {
    return !findPsExecPath().empty();
}

bool downloadPSTools() {
    string targetDir = getPSToolsDirectory();
    Logger::info("Starting PSTools download to: " + targetDir);

    cout << "\n========================================" << endl;
    cout << " Downloading PSTools from Sysinternals..." << endl;
    cout << " Target Directory: " << targetDir << endl;
    cout << "========================================\n" << endl;

    createFolderRecursive(targetDir);

    stringstream ss;
    ss << "powershell -Command \"";
    ss << "$ProgressPreference = 'SilentlyContinue'; ";
    ss << "Write-Host 'Downloading PSTools.zip...'; ";
    ss << "Invoke-WebRequest -Uri 'https://download.sysinternals.com/files/PSTools.zip' -OutFile '$env:TEMP\\PSTools.zip'; ";
    ss << "Write-Host 'Extracting archive to " << targetDir << "...'; ";
    ss << "Expand-Archive -Path '$env:TEMP\\PSTools.zip' -DestinationPath '" << targetDir << "' -Force; ";
    ss << "Remove-Item -Path '$env:TEMP\\PSTools.zip' -Force; ";
    ss << "Write-Host 'PSTools download complete!'-ForegroundColor Green\"";

    int exitCode = system(ss.str().c_str());

    setPsExecEulaAccepted();

    if (isPSToolsInstalled()) {
        Logger::success("PSTools installed successfully at: " + findPsExecPath());
        cout << "\n[+] PSTools successfully installed to: " << targetDir << endl;
        cout << "Press any key to continue...";
        _getch();
        return true;
    } else {
        Logger::error("Failed to verify PSTools installation. PowerShell exited with code: " + to_string(exitCode));
        cout << "\n[-] Error downloading PSTools. Please check your internet connection." << endl;
        cout << "Press any key to continue...";
        _getch();
        return false;
    }
}

bool runAsSystem(const string& command, bool waitForExit) {
    setPsExecEulaAccepted();

    string psExecPath = findPsExecPath();
    if (psExecPath.empty()) {
        cout << "\n[!] PSTools (PsExec.exe) is not installed." << endl;
        cout << "Do you want to download PSTools now? (y/n): ";
        char choice = _getch();
        cout << choice << endl;
        if (choice == 'y' || choice == 'Y') {
            if (!downloadPSTools()) {
                return false;
            }
            psExecPath = findPsExecPath();
        } else {
            return false;
        }
    }

    if (psExecPath.empty()) {
        Logger::error("Cannot execute command: PsExec not found.");
        return false;
    }

    string targetCmd = command;
    while (!targetCmd.empty() && (targetCmd.front() == ' ' || targetCmd.front() == '\t')) targetCmd.erase(0, 1);
    while (!targetCmd.empty() && (targetCmd.back() == ' ' || targetCmd.back() == '\t')) targetCmd.pop_back();

    // Auto-quote file path if path contains spaces and exists as a file but is not quoted
    if (!targetCmd.empty() && targetCmd.front() != '"' && targetCmd.find(' ') != string::npos) {
        if (GetFileAttributesA(targetCmd.c_str()) != INVALID_FILE_ATTRIBUTES) {
            targetCmd = "\"" + targetCmd + "\"";
        }
    }

    Logger::info("Launching command as SYSTEM: " + targetCmd);
    system("cls");
    cout << "========================================" << endl;
    cout << " Launching process as NT AUTHORITY\\SYSTEM" << endl;
    cout << " Command: " << targetCmd << endl;
    cout << " PsExec:  " << psExecPath << endl;
    cout << "========================================\n" << endl;

    string fullCmdLine = "\"" + psExecPath + "\" ";
    if (!waitForExit) {
        fullCmdLine += "-d ";
    }
    fullCmdLine += "-i -s -accepteula " + targetCmd;

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    vector<char> cmdBuffer(fullCmdLine.begin(), fullCmdLine.end());
    cmdBuffer.push_back('\0');

    DWORD creationFlags = waitForExit ? CREATE_NEW_CONSOLE : CREATE_NO_WINDOW;

    BOOL success = CreateProcessA(
        NULL,
        cmdBuffer.data(),
        NULL,
        NULL,
        TRUE,
        creationFlags,
        NULL,
        NULL,
        &si,
        &pi
    );

    DWORD exitCode = 0;
    if (success) {
        if (waitForExit) {
            WaitForSingleObject(pi.hProcess, INFINITE);
            GetExitCodeProcess(pi.hProcess, &exitCode);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
            Logger::info("PsExec process finished with exit code: " + to_string(exitCode));
            cout << "\n[+] PsExec completed with exit code: " << exitCode << endl;
            cout << "\nPress any key to return to menu...";
            _getch();
        } else {
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
            Logger::info("PsExec process launched asynchronously.");
        }
    } else {
        DWORD err = GetLastError();
        Logger::error("Failed to launch PsExec. CreateProcess error code: " + to_string(err));
        cout << "\n[-] Error launching PsExec. CreateProcess error code: " << err << endl;
        cout << "\nPress any key to return to menu...";
        _getch();
    }

    return (success && (waitForExit ? exitCode == 0 : true));
}

void showPSToolsMenu() {
    Logger::info("Opening PSTools / SYSTEM Launch Menu");

    vector<string> options = {
        "Launch 7SCW as SYSTEM",
        "CMD as SYSTEM (cmd.exe)",
        "PowerShell as SYSTEM (powershell.exe)",
        "Task Manager as SYSTEM (taskmgr.exe)",
        "Registry Editor as SYSTEM (regedit.exe)",
        "Custom Executable / Command as SYSTEM",
        "Download / Update PSTools",
        "Back to Main Menu"
    };

    int selectedIndex = 0;
    bool running = true;

    while (running) {
        system("cls");
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

        cout << "Run as SYSTEM (PSTools Manager)" << endl;
        cout << "Use Up/Down arrows to navigate, Enter to select, 'q' to back" << endl;

        if (isRunningAsSystem()) {
            SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
            cout << "Mode: Currently running as NT AUTHORITY\\SYSTEM!" << endl;
        }

        string currentPsExec = findPsExecPath();
        if (!currentPsExec.empty()) {
            SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
            cout << "Status: Installed -> " << currentPsExec << endl;
        } else {
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
            cout << "Status: Not Installed (Target: " << getPSToolsDirectory() << ")" << endl;
        }
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        cout << endl;

        for (size_t i = 0; i < options.size(); ++i) {
            bool isSel = (static_cast<int>(i) == selectedIndex);
            if (isSel) {
                SetConsoleTextAttribute(hConsole, BACKGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
            }

            if (options[i] == "Launch 7SCW as SYSTEM") {
                if (!isSel) {
                    SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                }
            } else if (options[i] == "Download / Update PSTools") {
                if (!isSel) {
                    SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                }
            }

            cout << (isSel ? "> " : "   ") << options[i] << endl;
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        }

        int key = _getch();
        if (key == 224) {
            key = _getch();
            switch (key) {
            case 72: // Up arrow
                selectedIndex = (selectedIndex - 1 + static_cast<int>(options.size())) % static_cast<int>(options.size());
                break;
            case 80: // Down arrow
                selectedIndex = (selectedIndex + 1) % static_cast<int>(options.size());
                break;
            }
        } else if (key == 13) { // Enter
            switch (selectedIndex) {
            case 0: { // Launch 7SCW as SYSTEM
                if (isRunningAsSystem()) {
                    cout << "\n[!] 7SCW is ALREADY running with NT AUTHORITY\\SYSTEM privileges!" << endl;
                    cout << "Press any key to return to menu...";
                    _getch();
                    break;
                }
                char buffer[MAX_PATH];
                DWORD len = GetModuleFileNameA(NULL, buffer, MAX_PATH);
                if (len > 0) {
                    string currentExePath = buffer;
                    if (runAsSystem("\"" + currentExePath + "\"", false)) {
                        cout << "\n[+] Successfully launched 7SCW as SYSTEM in a new window." << endl;
                        cout << "[+] Closing current process..." << endl;
                        Sleep(800);
                        exit(0);
                    }
                } else {
                    cout << "[-] Failed to resolve current executable path." << endl;
                    _getch();
                }
                break;
            }
            case 1: // CMD
                runAsSystem("cmd.exe");
                break;
            case 2: // PowerShell
                runAsSystem("powershell.exe");
                break;
            case 3: // Task Manager
                runAsSystem("taskmgr.exe");
                break;
            case 4: // Registry Editor
                runAsSystem("regedit.exe");
                break;
            case 5: { // Custom Executable / Command
                system("cls");
                cout << "========================================" << endl;
                cout << " Custom Executable / Command as SYSTEM" << endl;
                cout << "========================================\n" << endl;
                cout << "Enter executable path or command to run as SYSTEM:" << endl;
                cout << "Examples:" << endl;
                cout << "  - notepad.exe" << endl;
                cout << "  - C:\\Tools\\MyProgram.exe" << endl;
                cout << "  - \"C:\\Program Files\\App\\app.exe\" -arg1" << endl;
                cout << "\nPath/Command: ";

                string customCmd;
                getline(cin >> ws, customCmd);
                if (!customCmd.empty()) {
                    runAsSystem(customCmd);
                }
                break;
            }
            case 6: // Download / Update PSTools
                downloadPSTools();
                break;
            case 7: // Back
                running = false;
                break;
            }
        } else if (key == 'q' || key == 'Q' || key == 27) { // q, Q, or Esc
            running = false;
        }
    }
}
