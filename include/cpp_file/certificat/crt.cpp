// crt.cpp
#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <conio.h>
#include <algorithm>
#include "../../Logger.h"

using namespace std;

// Используем внешнюю переменную из main.cpp
extern volatile BOOL g_ctrlCPressed;

// --- Вспомогательные функции ---

bool FileExists(const string& path) {
    DWORD dwAttrib = GetFileAttributesA(path.c_str());
    return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

string ExecCommand(const string& cmd) {
    string result;
    char buffer[128];
    char tempPath[MAX_PATH];
    char tempFile[MAX_PATH];

    GetTempPathA(MAX_PATH, tempPath);
    GetTempFileNameA(tempPath, "scw", 0, tempFile);

    string fullCmd = "cmd /c \"" + cmd + " > \"" + string(tempFile) + "\" 2>&1\"";
    system(fullCmd.c_str());

    ifstream file(tempFile);
    if (file.is_open()) {
        while (file.getline(buffer, sizeof(buffer))) {
            result += buffer;
            result += "\n";
        }
        file.close();
    }
    DeleteFileA(tempFile);
    return result;
}

string FindSignTool() {
    vector<string> paths = {
        "C:\\Program Files (x86)\\Windows Kits\\10\\bin\\10.0.22621.0\\x64\\signtool.exe",
        "C:\\Program Files (x86)\\Windows Kits\\10\\bin\\10.0.19041.0\\x64\\signtool.exe",
        "C:\\Program Files (x86)\\Windows Kits\\10\\bin\\x64\\signtool.exe",
        "C:\\Program Files (x86)\\Microsoft SDKs\\Windows\\v7.1A\\Bin\\signtool.exe",
        "C:\\Program Files (x86)\\Windows Kits\\10\\App Certification Kit\\signtool.exe"
    };
    for (const auto& path : paths) {
        if (FileExists(path)) {
            Logger::info("SignTool found at: " + path);
            return path;
        }
    }
    Logger::warning("SignTool not found.");
    return "";
}

bool CreateCertificate(const string& name, const string& password, string& pfxPath) {
    if (g_ctrlCPressed) return false;
    
    char curDir[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, curDir);
    pfxPath = string(curDir) + "\\" + name + ".pfx";

    string escapedName = name;
    string escapedPass = password;
    size_t pos = 0;
    while ((pos = escapedName.find('"', pos)) != string::npos) {
        escapedName.replace(pos, 1, "`\"");
        pos += 2;
    }
    pos = 0;
    while ((pos = escapedPass.find('"', pos)) != string::npos) {
        escapedPass.replace(pos, 1, "`\"");
        pos += 2;
    }

    string psCommand = "powershell -ExecutionPolicy Bypass -Command \""
        "$Cert = New-SelfSignedCertificate -Type CodeSigningCert -Subject 'CN=" + escapedName + "' -CertStoreLocation 'Cert:\\CurrentUser\\My'; "
        "$CertPassword = ConvertTo-SecureString -String '" + escapedPass + "' -Force -AsPlainText; "
        "Export-PfxCertificate -Cert $Cert -FilePath '" + pfxPath + "' -Password $CertPassword -Force\"";

    Logger::info("Executing PowerShell to create certificate for: " + name);
    string output = ExecCommand(psCommand);
    if (g_ctrlCPressed) return false;
    
    if (output.find("Error") != string::npos || output.find("Exception") != string::npos) {
         Logger::error("PowerShell execution failed: " + output);
         return false;
    }

    bool success = FileExists(pfxPath);
    if (success) {
        Logger::success("PFX certificate created: " + pfxPath);
    } else {
        Logger::error("Failed to create PFX certificate.");
    }
    return success;
}

bool SignFile(const string& signtoolPath, const string& certPath, const string& password, const string& fileToSign) {
    if (g_ctrlCPressed) return false;
    
    if (!FileExists(signtoolPath)) {
        Logger::error("SignTool not found at: " + signtoolPath);
        return false;
    }
     if (!FileExists(certPath)) {
        Logger::error("Certificate not found: " + certPath);
        return false;
    }
    if (!FileExists(fileToSign)) {
        Logger::error("File to sign not found: " + fileToSign);
        return false;
    }

    string cmd = "\"" + signtoolPath + "\" sign /fd SHA256 /f \"" + certPath + "\" /p \"" + password + "\" /tr http://timestamp.digicert.com /td SHA256 \"" + fileToSign + "\"";

    Logger::info("Signing file: " + fileToSign);
    string output = ExecCommand(cmd);
    
    if (g_ctrlCPressed) return false;

    if (output.find("Successfully signed") != string::npos) {
        Logger::success("File successfully signed: " + fileToSign);
        return true;
    } else {
        Logger::error("Signing failed:\n" + output);
        return false;
    }
}

bool SafeGetLine(const string& prompt, string& output) {
    cout << prompt;
    output = "";
    
    while (true) {
        if (g_ctrlCPressed) {
            cout << "\n[^C] Operation cancelled" << endl;
            return false;
        }
        
        if (_kbhit()) {
            int ch = _getch();
            
            if (ch == 3) { // Ctrl+C
                cout << "\n[^C] Operation cancelled" << endl;
                return false;
            } else if (ch == 13) { // Enter
                cout << endl;
                return true;
            } else if (ch == 8) { // Backspace
                if (!output.empty()) {
                    output.pop_back();
                    cout << "\b \b";
                }
            } else if (ch >= 32 && ch <= 126) {
                output += ch;
                cout << (char)ch;
            }
        }
        
        if (g_ctrlCPressed) {
            cout << "\n[^C] Operation cancelled" << endl;
            return false;
        }
        
        Sleep(10);
    }
}

void DrawSignerMenu(const vector<string>& options, int selectedIndex, const string& signtoolPath) {
    system("cls");
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    int consoleWidth = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    
    string line(consoleWidth - 1, '=');
    cout << line << "\n";
    
		string title = "CERTIFICATE TOOL";
    int padding = (consoleWidth - (int)title.length()) / 2;
    cout << string(padding, ' ') << title << string(consoleWidth - padding - (int)title.length() - 1, ' ') << "\n";
    
    cout << line << "\n";
    
    if (signtoolPath.empty()) {
        cout << " STATUS: [!] SignTool NOT FOUND\n";
    } else {
        cout << " STATUS: [+] SignTool FOUND";
        string pathDisplay = signtoolPath;
        int maxPathLen = consoleWidth - 25;
        if ((int)pathDisplay.length() > maxPathLen) {
            pathDisplay = "..." + pathDisplay.substr(pathDisplay.length() - (maxPathLen - 3));
        }
        cout << "     PATH:   " << pathDisplay << "\n";
    }
    cout << line << "\n\n";
    
    for (int i = 0; i < (int)options.size(); i++) {
        bool isSel = (i == selectedIndex);
        if (isSel) {
            SetConsoleTextAttribute(hConsole, BACKGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
            cout << "> " << options[i];
        } else {
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            cout << "  " << options[i];
        }
        
        if (i == 0) cout << "  (Create new PFX and sign EXE)";
        else if (i == 1) cout << "  (Use existing PFX file)";
        else if (i == 2) cout << "  (Create PFX only)";
        cout << "\n";
    }
    
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    
    cout << "\n" << line << "\n";
    cout << "Press 'q' to return to main menu | Ctrl+C to cancel operation\n";
    cout << line << "\n";
}

// --- Основная функция меню подписи ---
void ShowFileSignerMenu() {
    vector<string> menuOptions = {
        "Create New Certificate & Sign EXE",
        "Sign EXE using existing PFX",
        "Just Generate New Certificate (PFX)",
        "Back to Main Menu"
    };
    
    int selectedIndex = 0;
    bool running = true;
    
    string signtoolPath = FindSignTool();
    
    while (running) {
        // Явно сбрасываем флаг при входе в меню
        g_ctrlCPressed = FALSE;
        
        DrawSignerMenu(menuOptions, selectedIndex, signtoolPath);
        
        int key = _getch();
        
        if (key == 224) {
            key = _getch();
            switch (key) {
                case 72:
                    selectedIndex = (selectedIndex - 1 + (int)menuOptions.size()) % (int)menuOptions.size();
                    break;
                case 80:
                    selectedIndex = (selectedIndex + 1) % (int)menuOptions.size();
                    break;
            }
        } else if (key == 13) {
            if (selectedIndex == 3) {
                running = false;
                continue;
            }
            
            system("cls");
            cout << "==================================================\n";
            
            string name, pass, exePath, pfxPath;
            bool operationCancelled = false;
            
            switch (selectedIndex) {
                case 0:
                    cout << ">>> CREATE NEW CERTIFICATE & SIGN EXE <<<\n";
                    cout << "==================================================\n\n";
                    cout << "(Press Ctrl+C at any time to cancel)\n\n";
                    
                    if (!SafeGetLine("Certificate Name: ", name)) { operationCancelled = true; break; }
                    if (g_ctrlCPressed) { operationCancelled = true; break; }
                    
                    if (!SafeGetLine("PFX Password: ", pass)) { operationCancelled = true; break; }
                    if (g_ctrlCPressed) { operationCancelled = true; break; }
                    
                    if (!SafeGetLine("EXE Path: ", exePath)) { operationCancelled = true; break; }
                    if (g_ctrlCPressed) { operationCancelled = true; break; }
                    
                    exePath.erase(remove(exePath.begin(), exePath.end(), '\"'), exePath.end());
                    
                    if (!g_ctrlCPressed && CreateCertificate(name, pass, pfxPath)) {
                        if (!g_ctrlCPressed && !signtoolPath.empty()) {
                            if (SignFile(signtoolPath, pfxPath, pass, exePath)) {
                                cout << "\n[+] SUCCESS: File signed successfully!\n";
                            } else if (!g_ctrlCPressed) {
                                cout << "\n[-] FAILED: Signing failed.\n";
                            }
                        } else if (!g_ctrlCPressed) {
                            cout << "\n[!] SignTool not found. Cannot sign file.\n";
                            cout << "Certificate was created at: " << pfxPath << "\n";
                        }
                    } else if (!g_ctrlCPressed) {
                        cout << "\n[-] FAILED: Certificate creation failed.\n";
                    }
                    break;
                    
                case 1:
                    cout << ">>> SIGN EXE USING EXISTING PFX <<<\n";
                    cout << "==================================================\n\n";
                    cout << "(Press Ctrl+C at any time to cancel)\n\n";
                    
                    if (signtoolPath.empty()) {
                        cout << "SignTool Path (or press Enter if not needed):\n";
                        string manualPath;
                        if (SafeGetLine("> ", manualPath) && !manualPath.empty()) {
                            manualPath.erase(remove(manualPath.begin(), manualPath.end(), '\"'), manualPath.end());
                            signtoolPath = manualPath;
                        } else {
                            operationCancelled = true;
                            break;
                        }
                    }
                    if (g_ctrlCPressed) { operationCancelled = true; break; }
                    
                    if (!SafeGetLine("PFX Path: ", pfxPath)) { operationCancelled = true; break; }
                    if (g_ctrlCPressed) { operationCancelled = true; break; }
                    
                    if (!SafeGetLine("Password: ", pass)) { operationCancelled = true; break; }
                    if (g_ctrlCPressed) { operationCancelled = true; break; }
                    
                    if (!SafeGetLine("EXE Path: ", exePath)) { operationCancelled = true; break; }
                    if (g_ctrlCPressed) { operationCancelled = true; break; }
                    
                    pfxPath.erase(remove(pfxPath.begin(), pfxPath.end(), '\"'), pfxPath.end());
                    exePath.erase(remove(exePath.begin(), exePath.end(), '\"'), exePath.end());
                    
                    if (!g_ctrlCPressed && !signtoolPath.empty()) {
                        if (SignFile(signtoolPath, pfxPath, pass, exePath)) {
                            cout << "\n[+] SUCCESS: File signed successfully!\n";
                        } else if (!g_ctrlCPressed) {
                            cout << "\n[-] FAILED: Signing failed.\n";
                        }
                    } else if (!g_ctrlCPressed) {
                        cout << "\n[!] SignTool path not specified. Cannot sign.\n";
                    }
                    break;
                    
                case 2:
                    cout << ">>> GENERATE NEW CERTIFICATE (PFX) <<<\n";
                    cout << "==================================================\n\n";
                    cout << "(Press Ctrl+C at any time to cancel)\n\n";
                    
                    if (!SafeGetLine("Certificate Name: ", name)) { operationCancelled = true; break; }
                    if (g_ctrlCPressed) { operationCancelled = true; break; }
                    
                    if (!SafeGetLine("PFX Password: ", pass)) { operationCancelled = true; break; }
                    if (g_ctrlCPressed) { operationCancelled = true; break; }
                    
                    if (!g_ctrlCPressed && CreateCertificate(name, pass, pfxPath)) {
                        cout << "\n[+] SUCCESS: Certificate created at: " << pfxPath << "\n";
                    } else if (!g_ctrlCPressed) {
                        cout << "\n[-] FAILED: Certificate creation failed.\n";
                    }
                    break;
            }
            
            if (operationCancelled || g_ctrlCPressed) {
                cout << "\n[!] Operation was cancelled by user.\n";
            }
            
            cout << "\n==================================================\n";
            cout << "Press any key to continue...";
            _getch();
            
            // Сбрасываем флаг после завершения операции
            g_ctrlCPressed = FALSE;
            
        } else if (key == 'q' || key == 'Q') {
            running = false;
        }
    }
}