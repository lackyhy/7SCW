#include <iostream>
#include <windows.h>
#include <vector>
#include <string>
#include <conio.h>
#include <fstream>
#include <direct.h>


class Menu {
private:
    std::vector<std::string> options;
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
        std::cout << "=== SCRIPT MANAGEMENT MENU ===\n\n";

        for (int i = 0; i < options.size(); ++i) {
            std::string optionText = options[i];
            // Remove leading \n if present and add it as a separate line
            if (optionText.substr(0, 1) == "\n") {
                optionText = optionText.substr(1); // Remove the \n
                std::cout << "\n"; // Add the newline before the option
            }
            
            if (i == selectedIndex) {
                setColor(FOREGROUND_BLUE | FOREGROUND_INTENSITY | BACKGROUND_INTENSITY);
                std::cout << " > " << optionText;
                int spaces = menuWidth - optionText.length();
                for (int j = 0; j < spaces; ++j) std::cout << " ";
                std::cout << " \n";
                resetColor();
            } else {
                setColor(7);
                std::cout << "   " << optionText << "\n";
            }
        }

        std::cout << "\n\nControls: Up/Down arrows - select, Enter - run, ESC - exit\n";
    }

public:
    Menu(const std::vector<std::string>& menuOptions) : options(menuOptions), selectedIndex(0) {
        menuWidth = 0;
        for (const auto& option : options) {
            if (option.length() > menuWidth) {
                menuWidth = option.length();
            }
        }
        menuWidth += 2;
    }

    int show() {
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

    std::string getSelectedOption() const {
        if (selectedIndex >= 0 && selectedIndex < options.size()) {
            return options[selectedIndex];
        }
        return "";
    }
};

bool confirmExecution(const std::string& scriptName) {
    Menu confirmMenu({"Yes, run script", "No, cancel"});

    std::cout << "Run script: " << scriptName << "?\n";
    int result = confirmMenu.show();

    return (result == 0);
}

std::vector<std::string> getGitHubScriptsList() {
    std::vector<std::string> scripts;

    std::cout << "Getting script list from GitHub...\n";

    system("curl -s https://api.github.com/repos/lackyhy/__script_ms_/contents/ > __temp_github_list.txt");

    std::ifstream file("__temp_github_list.txt");
    std::string line;

    while (std::getline(file, line)) {
        if (line.find("\"name\"") != std::string::npos) {
            size_t start = line.find(": \"") + 3;
            size_t end = line.find("\"", start);
            std::string fileName = line.substr(start, end - start);

            std::string extension = fileName.substr(fileName.find_last_of("."));
            if (extension == ".cmd" || extension == ".bat" || extension == ".ps1") {
                scripts.push_back(fileName);
            }
        }
    }

    file.close();
    remove("__temp_github_list.txt");

    return scripts;
}

bool downloadFromGitHub(const std::string& fileName) {
    std::string url = "https://raw.githubusercontent.com/lackyhy/__script_ms_/main/" + fileName;
    std::string localPath = "__script_/" + fileName;
    std::string command = "curl -s -o \"" + localPath + "\" \"" + url + "\"";

    std::cout << "Downloading: " << fileName << "\n";

    int result = system(command.c_str());

    if (result == 0) {
        std::cout << "Successfully downloaded: " << fileName << "\n";
        return true;
    } else {
        std::cout << "Failed to download: " << fileName << "\n";
        return false;
    }
}

std::vector<std::string> getAvailableScripts() {
    std::vector<std::string> scripts;

    WIN32_FIND_DATAA findFileData;
    HANDLE hFind = FindFirstFileA("__script_\\*", &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        return scripts;
    }

    do {
        if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            std::string fileName = findFileData.cFileName;
            std::string extension = fileName.substr(fileName.find_last_of("."));
            if (extension == ".cmd" || extension == ".bat" || extension == ".ps1") {
                scripts.push_back(fileName);
            }
        }
    } while (FindNextFileA(hFind, &findFileData) != 0);

    FindClose(hFind);
    return scripts;
}

bool executeScript(const std::string& scriptName) {
    std::string fullPath = "__script_\\" + scriptName;

    WIN32_FIND_DATAA findFileData;
    HANDLE hFind = FindFirstFileA(fullPath.c_str(), &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        std::cout << "Script not found: " << scriptName << "\n";
        std::cout << "Full path checked: " << fullPath << "\n";
        return false;
    }
    FindClose(hFind);

    std::string extension = scriptName.substr(scriptName.find_last_of("."));
    std::string command;

    if (extension == ".ps1") {
        command = "powershell -ExecutionPolicy Bypass -File \"" + fullPath + "\"";
    } else {
        command = "cd /d \"__script_\" && \"" + scriptName + "\"";
    }

    std::cout << "Executing: " << command << "\n";
    std::cout << "----------------------------------------\n";

    int result = system(command.c_str());

    std::cout << "----------------------------------------\n";
    std::cout << "Script finished with code: " << result << "\n";

    return (result == 0);
}

int showDownloadMenu() {
    std::vector<std::string> githubScripts = getGitHubScriptsList();

    if (githubScripts.empty()) {
        std::cout << "No scripts found on GitHub or failed to fetch list.\n";
        std::cout << "Press any key to continue...";
        _getch();
        return -1;
    }

    std::vector<std::string> downloadOptions;
    downloadOptions.push_back("Download all scripts");

    for (const auto& script : githubScripts) {
        downloadOptions.push_back(script);
    }

    downloadOptions.push_back("Cancel");

    Menu downloadMenu(downloadOptions);
    return downloadMenu.show();
}

void downloadScripts() {
    std::vector<std::string> githubScripts = getGitHubScriptsList();

    if (githubScripts.empty()) {
        std::cout << "No scripts available on GitHub.\n";
        std::cout << "Press any key to continue...";
        _getch();
        return;
    }

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
                std::cout << "All scripts downloaded successfully!\n";
            } else {
                std::cout << "Some scripts failed to download.\n";
            }

            std::cout << "Press any key to continue...";
            _getch();
            break;

        } else if (choice >= 1 && choice <= githubScripts.size()) {
            std::string selectedScript = githubScripts[choice - 1];

            if (downloadFromGitHub(selectedScript)) {
                std::cout << "Script downloaded successfully!\n";
            } else {
                std::cout << "Failed to download script.\n";
            }

            std::cout << "Press any key to continue...";
            _getch();
            break;
        }
    }
}

int showMainMenu() {
    std::vector<std::string> scripts = getAvailableScripts();
    std::vector<std::string> menuOptions;

    if (scripts.empty()) {
        menuOptions = {
                "\nDownload scripts from GitHub",
                "Exit"
        };
    } else {
        menuOptions = scripts;
        menuOptions.push_back("Download more scripts");
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
        std::vector<std::string> scripts = getAvailableScripts();
        int choice = showMainMenu();

        if (choice == -1) {
            std::cout << "Exiting program...\n";
            break;
        }

        if (scripts.empty()) {
            if (choice == 0) {
                downloadScripts();
            } else if (choice == 1) {
                std::cout << "Exiting program...\n";
                break;
            }
        } else {
            if (choice < scripts.size()) {
                std::string selectedScript = scripts[choice];

                if (confirmExecution(selectedScript)) {
                    std::cout << "Running script: " << selectedScript << "\n";
                    executeScript(selectedScript);

                    std::cout << "Press any key to continue...";
                    _getch();
                }
            } else if (choice == scripts.size()) {
                downloadScripts();
            } else if (choice == scripts.size() + 1) {
                std::cout << "Exiting program...\n";
                break;
            }
        }
    }
}