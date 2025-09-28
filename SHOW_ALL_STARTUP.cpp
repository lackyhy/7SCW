
#include <vector>
#include <string>
#include <conio.h>
#include <windows.h>
#include <algorithm>
#include <shlobj.h>
#include <iostream>
#include <fstream>
#include <map>
#include <sstream>

using namespace std;

struct StartupItem {
    string name;
    string path;
    string value;
    string type; // "registry", "folder", "task", "service"
    string location; // полный путь в реестре или файловой системе
    bool isMalicious; // подозрительный элемент
};

class StartupManager {
private:
    vector<StartupItem> items;
    int selectedIndex = 0;
    int scrollOffset = 0;
    const int MAX_VISIBLE_ITEMS = 20;

    // Функция для преобразования HKEY из строки
    HKEY getRootKey(const string& root) {
        if (root == "HKCU") return HKEY_CURRENT_USER;
        if (root == "HKLM") return HKEY_LOCAL_MACHINE;
        if (root == "HKCR") return HKEY_CLASSES_ROOT;
        if (root == "HKU") return HKEY_USERS;
        return NULL;
    }

    // Функция для разделения пути реестра на корневой ключ и подпуть
    bool parseRegistryPath(const string& fullPath, HKEY& rootKey, string& subPath) {
        size_t pos = fullPath.find('\\');
        if (pos == string::npos) return false;

        string root = fullPath.substr(0, pos);
        subPath = fullPath.substr(pos + 1);

        rootKey = getRootKey(root);
        return (rootKey != NULL);
    }

    // Функция для получения значений реестра с помощью WinAPI
    void collectRegistryValuesWinAPI(const string& regPath, const string& locationType) {
        HKEY rootKey;
        string subPath;

        if (!parseRegistryPath(regPath, rootKey, subPath)) {
            return;
        }

        HKEY hKey;
        if (RegOpenKeyExA(rootKey, subPath.c_str(), 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
            return;
        }

        DWORD index = 0;
        CHAR valueName[16383];
        DWORD valueNameSize;
        BYTE data[4096];
        DWORD dataSize;
        DWORD type;

        while (true) {
            valueNameSize = sizeof(valueName);
            dataSize = sizeof(data);

            LONG result = RegEnumValueA(hKey, index, valueName, &valueNameSize,
                                        NULL, &type, data, &dataSize);

            if (result == ERROR_NO_MORE_ITEMS) {
                break;
            }

            if (result == ERROR_SUCCESS) {
                StartupItem item;
                item.name = valueName;
                item.path = regPath;
                item.type = "registry";
                item.location = locationType;

                // Преобразуем данные в строку в зависимости от типа
                string valueStr;
                if (type == REG_SZ || type == REG_EXPAND_SZ) {
                    valueStr = string((CHAR*)data);
                } else if (type == REG_DWORD) {
                    DWORD dwValue = *(DWORD*)data;
                    valueStr = to_string(dwValue);
                } else if (type == REG_BINARY) {
                    valueStr = "[BINARY DATA]";
                } else {
                    valueStr = "[UNKNOWN TYPE]";
                }

                item.value = valueStr;
                item.isMalicious = checkIfMalicious(item);
                items.push_back(item);
            }

            index++;
        }

        RegCloseKey(hKey);
    }

    // Функция для выполнения команды и получения результата в string
    string executeCommand(const string& command) {
        string result;
        char buffer[128];

        // Добавляем перенаправление stderr в stdout
        string fullCommand = command + " 2>&1";
        FILE* pipe = _popen(fullCommand.c_str(), "r");
        if (!pipe) return "";

        while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
            result += buffer;
        }
        _pclose(pipe);
        return result;
    }

    // Функция для выполнения команды и разбора вывода построчно
    vector<string> executeCommandLines(const string& command) {
        vector<string> lines;
        string result = executeCommand(command);
        stringstream ss(result);
        string line;

        while (getline(ss, line)) {
            if (!line.empty()) {
                lines.push_back(line);
            }
        }
        return lines;
    }

    // Прогресс бар
    void showProgress(const string& message, int current, int total) {
        system("cls");
        cout << "========================================" << endl;
        cout << "    Collecting Startup Information" << endl;
        cout << "========================================" << endl << endl;

        cout << message << endl;

        int barWidth = 50;
        float progress = (float)current / total;
        int pos = barWidth * progress;

        cout << "[";
        for (int i = 0; i < barWidth; ++i) {
            if (i < pos) cout << "=";
            else if (i == pos) cout << ">";
            else cout << " ";
        }
        cout << "] " << int(progress * 100.0) << "%\r";
        cout.flush();
    }

    void collectStartupItems() {
        items.clear();

        vector<string> collectionSteps = {
                "Startup folders",
                "Registry startup",
                "Extended registry",
                "Scheduled tasks",
                "Windows services",
                "Browser extensions",
                "AppInit DLLs",
                "Winlogon items",
                "Policy items",
                "StartupApproved items"
        };

        int step = 0;
        int totalSteps = collectionSteps.size();

        showProgress("Initializing...", step, totalSteps);
        Sleep(100);

        // 1. Папки автозагрузки
        showProgress("Scanning startup folders...", ++step, totalSteps);
        collectFolderStartupItems();
        Sleep(100);

        // 2. Реестр - основные разделы
        showProgress("Scanning registry startup...", ++step, totalSteps);
        collectRegistryStartupItems();
        Sleep(100);

        // 3. Реестр - дополнительные разделы
        showProgress("Scanning extended registry...", ++step, totalSteps);
        collectExtendedRegistryItems();
        Sleep(100);

        // 4. Задачи планировщика
        showProgress("Scanning scheduled tasks...", ++step, totalSteps);
        collectScheduledTasks();
        Sleep(100);

        // 5. Службы
        showProgress("Scanning Windows services...", ++step, totalSteps);
        collectServices();
        Sleep(100);

        // 6. Browser Extensions
        showProgress("Scanning browser extensions...", ++step, totalSteps);
        collectBrowserExtensions();
        Sleep(100);

        // 7. AppInit DLLs
        showProgress("Scanning AppInit DLLs...", ++step, totalSteps);
        collectAppInitDLLs();
        Sleep(100);

        // 8. Winlogon
        showProgress("Scanning Winlogon items...", ++step, totalSteps);
        collectWinlogonItems();
        Sleep(100);

        // 9. Policies
        showProgress("Scanning policy items...", ++step, totalSteps);
        collectPolicyItems();
        Sleep(100);

        // 10. StartupApproved
        showProgress("Scanning StartupApproved items...", ++step, totalSteps);
        collectStartupApprovedItems();
        Sleep(100);

        // Сортировка по имени
        showProgress("Finalizing...", totalSteps, totalSteps);
        sort(items.begin(), items.end(), [](const StartupItem& a, const StartupItem& b) {
            return a.name < b.name;
        });

        Sleep(300);
    }

    void collectFolderStartupItems() {
        // Current User Startup Folder
        char appDataPath[MAX_PATH];
        if (SHGetFolderPathA(NULL, CSIDL_STARTUP, NULL, 0, appDataPath) == S_OK) {
            collectFilesFromFolder(string(appDataPath), "Startup Folder - Current User");
        }

        // All Users Startup Folder
        char commonStartupPath[MAX_PATH];
        if (SHGetFolderPathA(NULL, CSIDL_COMMON_STARTUP, NULL, 0, commonStartupPath) == S_OK) {
            collectFilesFromFolder(string(commonStartupPath), "Startup Folder - All Users");
        }
    }

    void collectFilesFromFolder(const string& folderPath, const string& locationType) {
        WIN32_FIND_DATAA findData;
        HANDLE hFind = FindFirstFileA((folderPath + "\\*").c_str(), &findData);

        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                if (strcmp(findData.cFileName, ".") != 0 && strcmp(findData.cFileName, "..") != 0) {
                    StartupItem item;
                    item.name = findData.cFileName;
                    item.path = folderPath + "\\" + findData.cFileName;
                    item.value = "";
                    item.type = "folder";
                    item.location = locationType;
                    item.isMalicious = checkIfMalicious(item);
                    items.push_back(item);
                }
            } while (FindNextFileA(hFind, &findData));
            FindClose(hFind);
        }
    }

    void collectRegistryStartupItems() {
        // Основные разделы автозагрузки - используем WinAPI
        vector<pair<string, string>> registryPaths = {
                {"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", "Registry - Current User Run"},
                {"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", "Registry - Local Machine Run"},
                {"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce", "Registry - Current User RunOnce"},
                {"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce", "Registry - Local Machine RunOnce"},
                {"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\RunServices", "Registry - Current User RunServices"},
                {"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\RunServices", "Registry - Local Machine RunServices"},
                {"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\RunServicesOnce", "Registry - Current User RunServicesOnce"},
                {"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\RunServicesOnce", "Registry - Local Machine RunServicesOnce"}
        };

        for (const auto& regPath : registryPaths) {
            collectRegistryValuesWinAPI(regPath.first, regPath.second);
        }
    }

    void collectExtendedRegistryItems() {
        // Расширенные разделы автозагрузки
        vector<pair<string, string>> extendedPaths = {
                {"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ShellExecuteHooks", "Registry - ShellExecuteHooks"},
                {"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\SharedTaskScheduler", "Registry - SharedTaskScheduler"},
                {"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Browser Helper Objects", "Registry - Browser Helper Objects"},
                {"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\Run", "Registry - Machine Policies"},
                {"HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\Run", "Registry - User Policies"},
                {"HKLM\\SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Run", "Registry - Wow6432Node Run"},
                {"HKCU\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Windows\\Run", "Registry - Windows Run"},
                {"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\Userinit", "Registry - Winlogon Userinit"},
                {"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\Shell", "Registry - Winlogon Shell"},
                {"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\ShellServiceObjectDelayLoad", "Registry - ShellServiceObjectDelayLoad"}
        };

        for (const auto& regPath : extendedPaths) {
            collectRegistryValuesWinAPI(regPath.first, regPath.second);
        }
    }

    void collectStartupApprovedItems() {
        // StartupApproved разделы
        vector<pair<string, string>> startupApprovedPaths = {
                {"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\Run", "StartupApproved - Machine Run"},
                {"HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\Run", "StartupApproved - User Run"},
                {"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\Run32", "StartupApproved - Machine Run32"},
                {"HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\Run32", "StartupApproved - User Run32"},
                {"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\StartupFolder", "StartupApproved - Machine StartupFolder"},
                {"HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\StartupFolder", "StartupApproved - User StartupFolder"},
                {"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\StartupFolderCommon", "StartupApproved - Machine StartupFolderCommon"},
                {"HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\StartupFolderCommon", "StartupApproved - User StartupFolderCommon"}
        };

        for (const auto& regPath : startupApprovedPaths) {
            collectRegistryValuesWinAPI(regPath.first, regPath.second);
        }
    }

    void collectScheduledTasks() {
        vector<string> taskKeywords = {"startup", "logon", "boot", "login"};

        for (const auto& keyword : taskKeywords) {
            string command = "schtasks /query /fo csv /nh | findstr /i \"" + keyword + "\"";
            vector<string> lines = executeCommandLines(command);

            for (const string& line : lines) {
                if (!line.empty()) {
                    size_t firstQuote = line.find("\"");
                    size_t secondQuote = line.find("\"", firstQuote + 1);

                    if (firstQuote != string::npos && secondQuote != string::npos) {
                        string taskName = line.substr(firstQuote + 1, secondQuote - firstQuote - 1);

                        StartupItem item;
                        item.name = taskName;
                        item.path = "Task Scheduler";
                        item.value = "";
                        item.type = "task";
                        item.location = "Scheduled Task - " + keyword;
                        item.isMalicious = checkIfMalicious(item);
                        items.push_back(item);
                    }
                }
            }
        }
    }

    void collectServices() {
        string command = "sc query type= service state= all | findstr \"SERVICE_NAME\"";
        vector<string> lines = executeCommandLines(command);

        for (const string& line : lines) {
            size_t pos = line.find("SERVICE_NAME:");
            if (pos != string::npos) {
                string serviceName = line.substr(pos + 13);
                serviceName.erase(0, serviceName.find_first_not_of(" \t"));
                serviceName.erase(serviceName.find_last_not_of(" \t") + 1);

                // Проверяем автозагрузку
                string checkCommand = "sc qc \"" + serviceName + "\" | findstr \"START_TYPE\"";
                string typeResult = executeCommand(checkCommand);

                if (typeResult.find("AUTO_START") != string::npos || typeResult.find("DEMAND_START") != string::npos) {
                    StartupItem item;
                    item.name = serviceName;
                    item.path = "Services";
                    item.value = "";
                    item.type = "service";
                    item.location = "Windows Services";
                    item.isMalicious = checkIfMalicious(item);
                    items.push_back(item);
                }
            }
        }
    }

    void collectBrowserExtensions() {
        vector<pair<string, string>> browserPaths = {
                {"HKLM\\SOFTWARE\\Microsoft\\Internet Explorer\\Extensions", "Browser - IE Extensions"},
                {"HKCU\\Software\\Microsoft\\Internet Explorer\\Extensions", "Browser - IE User Extensions"},
                {"HKLM\\SOFTWARE\\Google\\Chrome\\Extensions", "Browser - Chrome Extensions"},
                {"HKCU\\Software\\Google\\Chrome\\Extensions", "Browser - Chrome User Extensions"}
        };

        for (const auto& browserPath : browserPaths) {
            collectRegistryValuesWinAPI(browserPath.first, browserPath.second);
        }
    }

    void collectAppInitDLLs() {
        vector<pair<string, string>> appInitPaths = {
                {"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Windows\\AppInit_DLLs", "AppInit DLLs"},
                {"HKLM\\SOFTWARE\\Wow6432Node\\Microsoft\\Windows NT\\CurrentVersion\\Windows\\AppInit_DLLs", "AppInit DLLs Wow64"}
        };

        for (const auto& appInitPath : appInitPaths) {
            collectRegistryValuesWinAPI(appInitPath.first, appInitPath.second);
        }
    }

    void collectWinlogonItems() {
        vector<pair<string, string>> winlogonPaths = {
                {"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\Userinit", "Winlogon - Userinit"},
                {"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\Shell", "Winlogon - Shell"},
                {"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\System", "Winlogon - System"},
                {"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\Taskman", "Winlogon - Taskman"},
                {"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\AppSetup", "Winlogon - AppSetup"},
                {"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\Notify", "Winlogon - Notify"}
        };

        for (const auto& winlogonPath : winlogonPaths) {
            collectRegistryValuesWinAPI(winlogonPath.first, winlogonPath.second);
        }
    }

    void collectPolicyItems() {
        vector<pair<string, string>> policyPaths = {
                {"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\Run", "Policies - Machine Run"},
                {"HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\Run", "Policies - User Run"},
                {"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System\\Shell", "Policies - System Shell"}
        };

        for (const auto& policyPath : policyPaths) {
            collectRegistryValuesWinAPI(policyPath.first, policyPath.second);
        }
    }

    bool checkIfMalicious(const StartupItem& item) {
        vector<string> suspiciousKeywords = {
                "temp", "tmp", "appdata", "roaming", "microsoft",
                "update", "loader", "inject", "hook", "keylogger",
                "crypt", "miner", "bitcoin", "monero", "coinminer"
        };

        string lowerName = item.name;
        transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);

        for (const auto& keyword : suspiciousKeywords) {
            if (lowerName.find(keyword) != string::npos) {
                return true;
            }
        }

        return false;
    }
    void displayItems() {
        system("cls");
        cout << "========================================" << endl;
        cout << "    STARTUP MANAGER - All Locations" << endl;
        cout << "========================================" << endl << endl;

        cout << "Total items found: " << items.size() << endl;
        cout << "Use ARROW KEYS to navigate, ENTER to select, ESC to return" << endl << endl;

        int endIndex = min(scrollOffset + MAX_VISIBLE_ITEMS, (int)items.size());

        for (int i = scrollOffset; i < endIndex; i++) {
            if (i == selectedIndex) {
                cout << "> ";
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 14); // Желтый
            } else {
                cout << "  ";
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7); // Белый
            }

            // Подозрительные элементы помечаем красным
            if (items[i].isMalicious) {
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 12); // Красный
            }

            string displayName = items[i].name;
            if (displayName.length() > 35) {
                displayName = displayName.substr(0, 32) + "...";
            }

            cout << displayName;

            int spaces = 40 - displayName.length();
            if (spaces > 0) cout << string(spaces, ' ');

            cout << " [" << items[i].location << "]";

            if (items[i].isMalicious || i == selectedIndex) {
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7); // Возвращаем белый
            }
            cout << endl;
        }

        if (items.size() > MAX_VISIBLE_ITEMS) {
            cout << endl << "... " << (items.size() - endIndex) << " more items (Page Up/Down to scroll)" << endl;
        }

        cout << endl << "Legend: [RED] = Suspicious item" << endl;

        if (items.empty()) {
            cout << "No startup items found." << endl;
        }
    }

    void showItemDetails() {
        if (items.empty()) return;

        StartupItem& item = items[selectedIndex];

        system("cls");
        cout << "========================================" << endl;
        cout << "    STARTUP ITEM DETAILS" << endl;
        cout << "========================================" << endl << endl;

        cout << "Name: " << item.name << endl;
        cout << "Type: " << item.type << endl;
        cout << "Location: " << item.location << endl;

        if (item.type == "registry") {
            cout << "Registry Path: " << item.path << endl;
            cout << "Value: " << item.value << endl;
        } else if (item.type == "folder") {
            cout << "File Path: " << item.path << endl;
        } else if (item.type == "task") {
            cout << "Task Name: " << item.name << endl;
        } else if (item.type == "service") {
            cout << "Service Name: " << item.name << endl;
        }

        if (item.isMalicious) {
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 12);
            cout << "WARNING: This item appears suspicious!" << endl;
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
        }

        cout << endl << "ACTIONS:" << endl;
        cout << "1. Delete - Remove this startup item" << endl;
        cout << "2. Edit - Modify the startup item (registry only)" << endl;
        cout << "3. Open in Regedit - Open registry location" << endl;
        cout << "4. Open File Location - Open containing folder" << endl;
        cout << "5. Return - Go back to list" << endl;
        cout << endl << "Select action (1-5): ";
    }

    void deleteItem() {
        if (items.empty()) return;

        StartupItem& item = items[selectedIndex];

        cout << endl << "Are you sure you want to delete '" << item.name << "'? (y/n): ";
        char confirm;
        cin >> confirm;

        if (confirm == 'y' || confirm == 'Y') {
            bool success = false;

            if (item.type == "registry") {
                string command = "reg delete \"" + item.path + "\" /v \"" + item.name + "\" /f";
                success = (system(command.c_str()) == 0);
            } else if (item.type == "folder") {
                success = (DeleteFileA(item.path.c_str()) != 0);
            } else if (item.type == "task") {
                string command = "schtasks /delete /tn \"" + item.name + "\" /f";
                success = (system(command.c_str()) == 0);
            } else if (item.type == "service") {
                string command = "sc delete \"" + item.name + "\"";
                success = (system(command.c_str()) == 0);
            }

            if (success) {
                cout << "Item deleted successfully!" << endl;
                items.erase(items.begin() + selectedIndex);
                if (selectedIndex >= items.size()) selectedIndex = max(0, (int)items.size() - 1);
            } else {
                cout << "Failed to delete item!" << endl;
            }
        } else {
            cout << "Deletion cancelled." << endl;
        }

        cout << "Press any key to continue...";
        _getch();
    }

    void editItem() {
        if (items.empty()) return;

        StartupItem& item = items[selectedIndex];

        if (item.type == "registry") {
            cout << endl << "Current value: " << item.value << endl;
            cout << "Enter new value: ";
            string newValue;
            cin.ignore();
            getline(cin, newValue);

            if (!newValue.empty()) {
                string command = "reg add \"" + item.path + "\" /v \"" + item.name + "\" /t REG_SZ /d \"" + newValue + "\" /f";
                if (system(command.c_str()) == 0) {
                    item.value = newValue;
                    cout << "Value updated successfully!" << endl;
                } else {
                    cout << "Failed to update value!" << endl;
                }
            }
        } else {
            cout << "Editing is only available for registry items." << endl;
        }

        cout << "Press any key to continue...";
        _getch();
    }

    void openInRegedit() {
        if (items.empty()) return;

        StartupItem& item = items[selectedIndex];

        if (item.type == "registry") {
            string command = "regedit.exe";
            system(command.c_str());
            cout << "Regedit opened. Navigate to: " << item.path << "\\" << item.name << endl;
        } else {
            cout << "This action is only available for registry items." << endl;
        }

        cout << "Press any key to continue...";
        _getch();
    }

    void openFileLocation() {
        if (items.empty()) return;

        StartupItem& item = items[selectedIndex];

        if (item.type == "folder") {
            string command = "explorer.exe /select,\"" + item.path + "\"";
            system(command.c_str());
        } else if (item.type == "registry" && !item.value.empty()) {
            // Для реестровых элементов открываем папку, если значение является путем к файлу
            string filePath = item.value;
            // Убираем кавычки если есть
            if (filePath.front() == '"' && filePath.back() == '"') {
                filePath = filePath.substr(1, filePath.length() - 2);
            }
            // Извлекаем путь к директории
            size_t lastSlash = filePath.find_last_of("\\");
            if (lastSlash != string::npos) {
                string folderPath = filePath.substr(0, lastSlash);
                string command = "explorer.exe \"" + folderPath + "\"";
                system(command.c_str());
            }
        } else {
            cout << "File location is not available for this item type." << endl;
        }

        cout << "Press any key to continue...";
        _getch();
    }

    void handleItemActions() {
        while (true) {
            showItemDetails();

            char choice;
            cin >> choice;

            switch (choice) {
                case '1':
                    deleteItem();
                    return;
                case '2':
                    editItem();
                    return;
                case '3':
                    openInRegedit();
                    return;
                case '4':
                    openFileLocation();
                    return;
                case '5':
                    return;
                default:
                    cout << "Invalid choice! Press any key to continue...";
                    _getch();
                    break;
            }
        }
    }

public:
    void SHOW_ALL_STARTUP() {
        collectStartupItems();

        if (items.empty()) {
            system("cls");
            cout << "No startup items found." << endl;
            cout << "Press any key to continue...";
            _getch();
            return;
        }

        while (true) {
            displayItems();

            int key = _getch();
            if (key == 0 || key == 224) {
                key = _getch();
                switch (key) {
                    case 72: // UP
                        selectedIndex = max(0, selectedIndex - 1);
                        if (selectedIndex < scrollOffset) scrollOffset = selectedIndex;
                        break;
                    case 80: // DOWN
                        selectedIndex = min((int)items.size() - 1, selectedIndex + 1);
                        if (selectedIndex >= scrollOffset + MAX_VISIBLE_ITEMS) {
                            scrollOffset = selectedIndex - MAX_VISIBLE_ITEMS + 1;
                        }
                        break;
                    case 73: // PAGE UP
                        selectedIndex = max(0, selectedIndex - MAX_VISIBLE_ITEMS);
                        scrollOffset = max(0, scrollOffset - MAX_VISIBLE_ITEMS);
                        break;
                    case 81: // PAGE DOWN
                        selectedIndex = min((int)items.size() - 1, selectedIndex + MAX_VISIBLE_ITEMS);
                        scrollOffset = min((int)items.size() - MAX_VISIBLE_ITEMS, scrollOffset + MAX_VISIBLE_ITEMS);
                        break;
                }
            } else {
                switch (key) {
                    case 13: // ENTER
                        handleItemActions();
                        break;
                    case 27: // ESC
                        return;
                    case 'r': case 'R': // Refresh
                        collectStartupItems();
                        break;
                }
            }
        }
    }
};

StartupManager startupManager;

void startupMG_SHOW_ALL_STARTUP() {
    startupManager.SHOW_ALL_STARTUP();
}