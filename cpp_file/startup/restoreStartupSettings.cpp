#include "iostream"
#include "string"

#include "algorithm"
#include "conio.h"
#include "fstream"

using namespace std;

void restoreStartupSettings() {
    system("cls");
    cout << "========================================" << endl;
    cout << "   Restore to Original" << endl;
    cout << "========================================" << endl << endl;

    cout << "This will restore startup settings to their original state." << endl;
    cout << "WARNING: This may affect system startup behavior!" << endl << endl;

    // Показать конкретные записи реестра которые будут удалены
    cout << "The following specific registry entries and tasks will be DELETED:" << endl;
    cout << "==================================================================" << endl << endl;

    cout << "REGISTRY ENTRIES:" << endl;
    cout << "-----------------" << endl;

    // Функция для вывода текущих значений реестра
    auto showRegistryValues = [](const string& regPath, const string& description = "") {
        cout << "• " << regPath;
        if (!description.empty()) {
            cout << " (" << description << ")";
        }
        cout << endl;

        // Создаем временный файл для вывода reg query
        string tempFile = "temp_reg_output.txt";
        string command = "reg query \"" + regPath + "\" 2>nul > " + tempFile;
        system(command.c_str());

        // Читаем и выводим содержимое
        ifstream file(tempFile);
        string line;
        bool hasContent = false;

        while (getline(file, line)) {
            if (!line.empty() && line.find(regPath) == string::npos) {
                if (!hasContent) {
                    cout << "\t\tCurrent values:" << endl;
                    hasContent = true;
                }
                cout << "\t\t  " << line << endl;
            }
        }
        file.close();

        // Удаляем временный файл
        remove(tempFile.c_str());

        if (!hasContent) {
            cout << "\t\t(No values found)" << endl;
        }
        cout << endl;
    };

    // Функция для вывода значений конкретных ключей
    auto showRegistryKeyValues = [](const string& regPath, const string& valueName = "") {
        cout << "• " << regPath;
        if (!valueName.empty()) {
            cout << "\\" << valueName;
        }
        cout << endl;

        string tempFile = "temp_reg_output.txt";
        string command;
        if (valueName.empty()) {
            command = "reg query \"" + regPath + "\" 2>nul > " + tempFile;
        } else {
            command = "reg query \"" + regPath + "\" /v \"" + valueName + "\" 2>nul > " + tempFile;
        }
        system(command.c_str());

        ifstream file(tempFile);
        string line;
        bool hasContent = false;

        while (getline(file, line)) {
            if (!line.empty() && line.find(regPath) == string::npos) {
                if (!hasContent) {
                    cout << "\t\tCurrent value: ";
                    hasContent = true;
                }
                cout << line << endl;
            }
        }
        file.close();

        remove(tempFile.c_str());

        if (!hasContent) {
            cout << "\t\t(Value not set)" << endl;
        }
        cout << endl;
    };

    // Вывод всех записей с текущими значениями
    showRegistryValues("HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", "User startup programs");
    showRegistryValues("HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", "Machine startup programs");
    showRegistryValues("HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce", "User run once programs");
    showRegistryValues("HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce", "Machine run once programs");
    showRegistryValues("HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\RunServices", "User services");
    showRegistryValues("HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\RunServices", "Machine services");
    showRegistryValues("HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\RunServicesOnce", "User run once services");
    showRegistryValues("HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\RunServicesOnce", "Machine run once services");
    showRegistryValues("HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ShellExecuteHooks", "Shell execute hooks");
    showRegistryValues("HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\SharedTaskScheduler", "Shared tasks");
    showRegistryValues("HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Browser Helper Objects", "Browser helpers");
    showRegistryValues("HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\Run", "Machine policies");
    showRegistryValues("HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\Run", "User policies");
    showRegistryValues("HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\Run", "Machine approved startup");
    showRegistryValues("HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\Run", "User approved startup");
    showRegistryValues("HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\Run32", "Machine 32-bit approved");
    showRegistryValues("HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\Run32", "User 32-bit approved");
    showRegistryValues("HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\StartupFolder", "Machine startup folder");
    showRegistryValues("HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\StartupFolder", "User startup folder");
    showRegistryValues("HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\StartupFolderCommon", "Machine common startup");
    showRegistryValues("HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\StartupFolderCommon", "User common startup");

    // ВАЖНО: Не показываем и не удаляем HKLM\SYSTEM\CurrentControlSet\Services - это критически важно!
    cout << "• HKLM\\SYSTEM\\CurrentControlSet\\Services (System services)" << endl;
    cout << "\t\t(SKIPPED - Contains critical system drivers and services)" << endl << endl;

    cout << "WINLOGON VALUES:" << endl;
    cout << "----------------" << endl;
    showRegistryKeyValues("HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon", "Taskman");
    showRegistryKeyValues("HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon", "AppSetup");
    showRegistryKeyValues("HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon", "System");

    cout << "TASK SCHEDULER TASKS:" << endl;
    cout << "---------------------" << endl;
    cout << "• All tasks containing 'startup' in name" << endl;
    // Показ задач планировщика
    system("schtasks /query /fo list | findstr /i \"startup\" > temp_tasks.txt 2>nul");
    ifstream taskFile("temp_tasks.txt");
    string taskLine;
    bool hasTasks = false;
    cout << "\t\tCurrent tasks:" << endl;
    while (getline(taskFile, taskLine)) {
        if (!taskLine.empty()) {
            cout << "\t\t  " << taskLine << endl;
            hasTasks = true;
        }
    }
    taskFile.close();
    remove("temp_tasks.txt");
    if (!hasTasks) {
        cout << "\t\t  (No startup tasks found)" << endl;
    }
    cout << endl;

    cout << "• All tasks containing 'logon' in name" << endl;
    system("schtasks /query /fo list | findstr /i \"logon\" > temp_tasks.txt 2>nul");
    taskFile.open("temp_tasks.txt");
    hasTasks = false;
    cout << "\t\tCurrent tasks:" << endl;
    while (getline(taskFile, taskLine)) {
        if (!taskLine.empty()) {
            cout << "\t\t  " << taskLine << endl;
            hasTasks = true;
        }
    }
    taskFile.close();
    remove("temp_tasks.txt");
    if (!hasTasks) {
        cout << "\t\t  (No logon tasks found)" << endl;
    }
    cout << endl;

    cout << "• All tasks containing 'boot' in name" << endl;
    cout << "\t\t(SKIPPED - Contains critical system boot tasks)" << endl << endl;

    cout << "DEFAULT VALUES WILL BE RESTORED:" << endl;
    cout << "--------------------------------" << endl;
    showRegistryKeyValues("HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon", "Shell");
    showRegistryKeyValues("HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon", "Userinit");

    cout << "==================================================================" << endl << endl;

    // Запрос подтверждения
    cout << "ARE YOU SURE YOU WANT TO DELETE ALL THESE ENTRIES? (y/n): ";
    char choice;
    cin >> choice;

    if (choice != 'y' && choice != 'Y') {
        cout << "Operation cancelled by user." << endl;
        cout << endl << "Press any key to continue...";
        _getch();
        return;
    }

    cout << endl << "Starting restoration process..." << endl << endl;

    // Restore default registry values
    cout << "[1/7] Restoring default registry values..." << endl;
    cout << "  Setting: HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\Shell = explorer.exe" << endl;
    system("reg add \"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\" /v Shell /t REG_SZ /d explorer.exe /f");
    cout << "  Setting: HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\Userinit = userinit.exe," << endl;
    system("reg add \"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\" /v Userinit /t REG_SZ /d \"C:\\Windows\\system32\\userinit.exe,\" /f");
    cout << "  ✓ Default registry values restored" << endl << endl;

    // Clear ALL registry startup entries
    cout << "[2/7] Clearing ALL registry startup entries..." << endl;
    cout << "  Deleting: HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Run" << endl;
    system("reg delete \"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Run\" /f");
    cout << "  Deleting: HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Run" << endl;
    system("reg delete \"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Run\" /f");
    cout << "  Deleting: HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce" << endl;
    system("reg delete \"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce\" /f");
    cout << "  Deleting: HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce" << endl;
    system("reg delete \"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce\" /f");
    cout << "  Deleting: HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\RunServices" << endl;
    system("reg delete \"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\RunServices\" /f");
    cout << "  Deleting: HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\RunServices" << endl;
    system("reg delete \"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\RunServices\" /f");
    cout << "  Deleting: HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\RunServicesOnce" << endl;
    system("reg delete \"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\RunServicesOnce\" /f");
    cout << "  Deleting: HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\RunServicesOnce" << endl;
    system("reg delete \"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\RunServicesOnce\" /f");
    cout << "  ✓ Registry startup entries cleared" << endl << endl;

    // Clear ShellExecuteHooks and other startup locations
    cout << "[3/7] Clearing ShellExecuteHooks and Browser Helper Objects..." << endl;
    cout << "  Deleting: HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ShellExecuteHooks" << endl;
    system("reg delete \"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ShellExecuteHooks\" /f");
    cout << "  Deleting: HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\SharedTaskScheduler" << endl;
    system("reg delete \"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\SharedTaskScheduler\" /f");
    cout << "  Deleting: HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Browser Helper Objects" << endl;
    system("reg delete \"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Browser Helper Objects\" /f");
    cout << "  ✓ ShellExecuteHooks and BHO cleared" << endl << endl;

    // Clear additional startup locations
    cout << "[4/7] Clearing additional startup locations..." << endl;
    cout << "  Deleting: HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\Run" << endl;
    system("reg delete \"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\Run\" /f");
    cout << "  Deleting: HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\Run" << endl;
    system("reg delete \"HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\Run\" /f");
    cout << "  Deleting: HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\Run" << endl;
    system("reg delete \"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\Run\" /f");
    cout << "  Deleting: HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\Run" << endl;
    system("reg delete \"HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\Run\" /f");
    cout << "  Deleting: HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\Run32" << endl;
    system("reg delete \"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\Run32\" /f");
    cout << "  Deleting: HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\Run32" << endl;
    system("reg delete \"HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\Run32\" /f");
    cout << "  ✓ Additional startup locations cleared" << endl << endl;

    // Clear Task Scheduler startup tasks (без boot задач)
    cout << "[5/7] Clearing Task Scheduler startup tasks..." << endl;
    cout << "  Deleting tasks with: *startup*" << endl;
    system("schtasks /delete /tn \"*startup*\" /f");
    cout << "  Deleting tasks with: *logon*" << endl;
    system("schtasks /delete /tn \"*logon*\" /f");
    cout << "  Skipping tasks with: *boot* (critical system tasks)" << endl;
    cout << "  ✓ Task Scheduler startup tasks cleared" << endl << endl;

    // ПРОПУСКАЕМ удаление системных служб - это критически важно!
    cout << "[6/7] Checking services startup entries..." << endl;
    cout << "  SKIPPING: HKLM\\SYSTEM\\CurrentControlSet\\Services" << endl;
    cout << "  This section contains critical system drivers and services" << endl;
    cout << "  Deleting it would make the system unbootable!" << endl;
    cout << "  ✓ Services section preserved (system stability)" << endl << endl;

    // Clear additional startup locations
    cout << "[7/7] Clearing additional startup locations..." << endl;
    cout << "  Deleting: HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\StartupFolder" << endl;
    system("reg delete \"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\StartupFolder\" /va /f 2>nul");
    cout << "  Deleting: HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\StartupFolder" << endl;
    system("reg delete \"HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\StartupFolder\" /va /f 2>nul");
    cout << "  Deleting: HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\StartupFolderCommon" << endl;
    system("reg delete \"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\StartupFolderCommon\" /va /f 2>nul");
    cout << "  Deleting: HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\StartupFolderCommon" << endl;
    system("reg delete \"HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\StartupFolderCommon\" /va /f 2>nul");


    cout << "  ✓ Additional startup locations cleared" << endl << endl;

    cout << "========================================" << endl;
    cout << "ALL startup entries cleared successfully!" << endl;
    cout << "System-critical entries were preserved for stability." << endl;
    cout << "========================================" << endl;

    cout << endl << "Press any key to continue...";
    _getch();
}
