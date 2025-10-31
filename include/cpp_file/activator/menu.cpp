#include <iostream>
#include <windows.h>
#include <vector>
#include <string>
#include <conio.h>
#include <fstream>
#include <direct.h>
#include "../../Logger.h"

using namespace std;

class Menu {
private:
    vector<string> options;
    int selectedIndex;
    int menuWidth;

    void clearScreen() {
        system("cls");
    }

    void setColor(int color) {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
    }

    void resetColor() {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
    }

    void drawMenu() {
        clearScreen();
        cout << "=== SCRIPT MANAGEMENT MENU ===\n\n";

        for (int i = 0; i < options.size(); ++i) {
            string optionText = options[i];
            // Remove leading \n if present and add it as a separate line
            if (optionText.substr(0, 1) == "\n") {
                optionText = optionText.substr(1); // Remove the \n
                cout << "\n"; // Add the newline before the option
            }
            
            if (i == selectedIndex) {
                setColor(FOREGROUND_BLUE | FOREGROUND_INTENSITY | BACKGROUND_INTENSITY);
                cout << " > " << optionText;
                int spaces = menuWidth - optionText.length();
                for (int j = 0; j < spaces; ++j) cout << " ";
                cout << " \n";
                resetColor();
            } else {
                setColor(7);
                cout << "   " << optionText << "\n";
            }
        }

        cout << "\n\nControls: Up/Down arrows - select, Enter - run, ESC - exit\n";
    }

public:
    Menu(const vector<string>& menuOptions) : options(menuOptions), selectedIndex(0) {
        menuWidth = 0;
        for (const auto& option : options) {
            if (option.length() > menuWidth) {
                menuWidth = option.length();
            }
        }
        menuWidth += 2;
    }

    int show() {
        Logger::info("Showing activator menu");
        while (true) {
            drawMenu();

            int key = _getch();

            if (key == 224 || key == 0) {
                key = _getch();
                switch (key) {
                    case 72:
                        selectedIndex = (selectedIndex - 1 + options.size()) % options.size();
                        break;
                    case 80:
                        selectedIndex = (selectedIndex + 1) % options.size();
                        break;
                }
            }
            else if (key == 13) {
                return selectedIndex;
            }
            else if (key == 27) {
                return -1;
            }
            else if (key >= '1' && key <= '9') {
                int num = key - '1';
                if (num < options.size()) {
                    selectedIndex = num;
                    return selectedIndex;
                }
            }
        }
    }

    string getSelectedOption() const {
        if (selectedIndex >= 0 && selectedIndex < options.size()) {
            return options[selectedIndex];
        }
        return "";
    }
};

bool confirmExecution(const string& scriptName) {
    Menu confirmMenu({"Yes, run script", "No, cancel"});

    cout << "Run script: " << scriptName << "?\n";
    int result = confirmMenu.show();

    return (result == 0);
}

vector<string> getGitHubScriptsList() {
    vector<string> scripts;
    Logger::functions_log(LOG_INFO, "getGitHubScriptsList", "Getting script list from GitHub...");
    cout << "Getting script list from GitHub...\n";

    system("curl -s https://api.github.com/repos/lackyhy/__script_ms_/contents/ > __temp_github_list.txt");

    ifstream file("__temp_github_list.txt");
    string line;

    while (getline(file, line)) {
        if (line.find("\"name\"") != string::npos) {
            size_t start = line.find(": \"") + 3;
            size_t end = line.find("\"", start);
            string fileName = line.substr(start, end - start);

            string extension = fileName.substr(fileName.find_last_of("."));
            if (extension == ".cmd" || extension == ".bat" || extension == ".ps1") {
                scripts.push_back(fileName);
            }
        }
    }

    file.close();
    remove("__temp_github_list.txt");

    return scripts;
}

bool downloadFromGitHub(const string& fileName) {
    Logger::functions_log(LOG_INFO, "downloadFromGitHub", "starting download script");
    string url = "https://raw.githubusercontent.com/lackyhy/__script_ms_/main/" + fileName;
    string localPath = "__script_/" + fileName;
    string command = "curl -s -o \"" + localPath + "\" \"" + url + "\"";

    cout << "Downloading: " << fileName << "\n";
    Logger::functions_log(LOG_INFO, "downloadFromGitHub", "Download script: " + fileName);
    int result = system(command.c_str());

    if (result == 0) {
        cout << "Successfully downloaded: " << fileName << "\n";
        Logger::functions_log(LOG_INFO, "downloadFromGitHub", "Successfully downloaded: " + fileName);
        return true;
    } else {
        cout << "Failed to download: " << fileName << "\n";
        Logger::functions_log(LOG_ERROR, "downloadFromGitHub", "Failed to download: " + fileName);
        return false;
    }
}

vector<string> getAvailableScripts() {
    Logger::functions_log(LOG_INFO, "getAvailableScripts", "");
    vector<string> scripts;

    WIN32_FIND_DATAA findFileData;
    HANDLE hFind = FindFirstFileA("__script_\\*", &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        return scripts;
    }

    do {
        if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            string fileName = findFileData.cFileName;
            string extension = fileName.substr(fileName.find_last_of("."));
            if (extension == ".cmd" || extension == ".bat" || extension == ".ps1") {
                scripts.push_back(fileName);
            }
        }
    } while (FindNextFileA(hFind, &findFileData) != 0);

    FindClose(hFind);
    return scripts;
}

bool executeScript(const string& scriptName) {
    string fullPath = "__script_\\" + scriptName;

    WIN32_FIND_DATAA findFileData;
    HANDLE hFind = FindFirstFileA(fullPath.c_str(), &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        cout << "Script not found: " << scriptName << "\n";
        cout << "Full path checked: " << fullPath << "\n";
        return false;
    }
    FindClose(hFind);

    string extension = scriptName.substr(scriptName.find_last_of("."));
    string command;

    if (extension == ".ps1") {
        command = "powershell -ExecutionPolicy Bypass -File \"" + fullPath + "\"";
    } else {
        command = "cd /d \"__script_\" && \"" + scriptName + "\"";
    }

    cout << "Executing: " << command << "\n";
    cout << "----------------------------------------\n";

    int result = system(command.c_str());

    cout << "----------------------------------------\n";
    cout << "Script finished with code: " << result << "\n";

    return (result == 0);
}

int showDownloadMenu() {
    vector<string> githubScripts = getGitHubScriptsList();

    if (githubScripts.empty()) {
        cout << "No scripts found on GitHub or failed to fetch list.\n";
        cout << "Press any key to continue...";
        _getch();
        return -1;
    }

    vector<string> downloadOptions;
    downloadOptions.push_back("Download all scripts");

    for (const auto& script : githubScripts) {
        downloadOptions.push_back(script);
    }

    downloadOptions.push_back("Cancel");

    Menu downloadMenu(downloadOptions);
    return downloadMenu.show();
}

void downloadScripts() {
    Logger::functions_log(LOG_INFO, "downloadScripts", "Starting script download process");
    vector<string> githubScripts = getGitHubScriptsList();

    if (githubScripts.empty()) {
        Logger::warning("No scripts available on GitHub");
        cout << "No scripts available on GitHub.\n";
        cout << "Press any key to continue...";
        _getch();
        return;
    }
    
    Logger::info("Found " + to_string(githubScripts.size()) + " scripts available for download");

    while (true) {
        int choice = showDownloadMenu();

        if (choice == -1 || choice == githubScripts.size() + 1) {
            break;
        }

        if (choice == 0) {
            bool allSuccess = true;
            for (const auto& script : githubScripts) {
                if (!downloadFromGitHub(script)) {
                    allSuccess = false;
                }
            }

            if (allSuccess) {
                cout << "All scripts downloaded successfully!\n";
            } else {
                cout << "Some scripts failed to download.\n";
            }

            cout << "Press any key to continue...";
            _getch();
            break;

        } else if (choice >= 1 && choice <= githubScripts.size()) {
            string selectedScript = githubScripts[choice - 1];

            if (downloadFromGitHub(selectedScript)) {
                cout << "Script downloaded successfully!\n";
            } else {
                cout << "Failed to download script.\n";
            }

            cout << "Press any key to continue...";
            _getch();
            break;
        }
    }
}

int showMainMenu() {
    vector<string> scripts = getAvailableScripts();
    vector<string> menuOptions;

    if (scripts.empty()) {
        menuOptions = {
                "\nDownload scripts from GitHub",
                "Exit"
        };
    } else {
        menuOptions = scripts;
        menuOptions.push_back("\nDownload more scripts");
        menuOptions.push_back("Exit");
    }

    Menu menu(menuOptions);
    return menu.show();
}

void __main_() {
    SetConsoleOutputCP(65001);

    if (_access("__script_", 0) != 0) {
        _mkdir("__script_");
    }

    while (true) {
        vector<string> scripts = getAvailableScripts();
        int choice = showMainMenu();

        if (choice == -1) {
            cout << "Exiting program...\n";
            break;
        }

        if (scripts.empty()) {
            if (choice == 0) {
                downloadScripts();
            } else if (choice == 1) {
                cout << "Exiting program...\n";
                break;
            }
        } else {
            if (choice < scripts.size()) {
                string selectedScript = scripts[choice];

                if (confirmExecution(selectedScript)) {
                    cout << "Running script: " << selectedScript << "\n";
                    executeScript(selectedScript);

                    cout << "Press any key to continue...";
                    _getch();
                }
            } else if (choice == scripts.size()) {
                downloadScripts();
            } else if (choice == scripts.size() + 1) {
                cout << "Exiting program...\n";
                break;
            }
        }
    }
}