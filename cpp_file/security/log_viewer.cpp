#include "../../h_file/security/log_viewer.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <conio.h>

using namespace std;

LogViewer::LogViewer() {
    // Инициализация фильтра по умолчанию
    currentFilter.showErrors = true;
    currentFilter.showWarnings = true;
    currentFilter.showInfo = false;
}

void LogViewer::readEventLog(const string& logName) {
    HANDLE hEventLog = OpenEventLogA(NULL, logName.c_str());
    if (hEventLog == NULL) {
        cout << "Error: Cannot open event log " << logName << endl;
        return;
    }
    
    DWORD dwBytesRead, dwMinNumberOfBytesNeeded;
    char buffer[4096];
    
    while (ReadEventLogA(hEventLog, EVENTLOG_BACKWARDS_READ | EVENTLOG_SEQUENTIAL_READ,
                        0, buffer, sizeof(buffer), &dwBytesRead, &dwMinNumberOfBytesNeeded)) {
        
        PEVENTLOGRECORD pRecord = (PEVENTLOGRECORD)buffer;
        
        while ((char*)pRecord < buffer + dwBytesRead) {
            parseEventLogEntry(pRecord);
            
            // Переход к следующей записи
            pRecord = (PEVENTLOGRECORD)((char*)pRecord + pRecord->Length);
        }
    }
    
    CloseEventLog(hEventLog);
}

void LogViewer::parseEventLogEntry(PEVENTLOGRECORD pRecord) {
    LogEntry entry;
    
    // Извлекаем информацию из записи
    entry.eventId = pRecord->EventID;
    entry.computer = (char*)((char*)pRecord + sizeof(EVENTLOGRECORD));
    
    // Получаем имя источника
    const char* sourceName = entry.computer.c_str() + entry.computer.length() + 1;
    entry.source = sourceName;
    
    // Получаем категорию
    const char* category = sourceName + strlen(sourceName) + 1;
    entry.category = category;
    
    // Получаем описание
    const char* description = category + strlen(category) + 1;
    entry.description = description;
    
    // Преобразуем время
    FILETIME ft;
    SYSTEMTIME st;
    ft.dwLowDateTime = pRecord->TimeGenerated;
    ft.dwHighDateTime = 0;
    FileTimeToSystemTime(&ft, &st);
    
    stringstream ss;
    ss << setfill('0') << setw(2) << st.wDay << "/"
       << setw(2) << st.wMonth << "/" << st.wYear << " "
       << setw(2) << st.wHour << ":" << setw(2) << st.wMinute << ":" << setw(2) << st.wSecond;
    entry.timestamp = ss.str();
    
    // Определяем уровень
    switch (pRecord->EventType) {
        case EVENTLOG_ERROR_TYPE:
            entry.level = "ERROR";
            break;
        case EVENTLOG_WARNING_TYPE:
            entry.level = "WARNING";
            break;
        case EVENTLOG_INFORMATION_TYPE:
            entry.level = "INFO";
            break;
        case EVENTLOG_AUDIT_SUCCESS:
            entry.level = "SUCCESS";
            break;
        case EVENTLOG_AUDIT_FAILURE:
            entry.level = "FAILURE";
            break;
        default:
            entry.level = "UNKNOWN";
            break;
    }
    
    logEntries.push_back(entry);
}

bool LogViewer::matchesFilter(const LogEntry& entry) {
    // Проверка источника
    if (!currentFilter.source.empty() && entry.source.find(currentFilter.source) == string::npos) {
        return false;
    }
    
    // Проверка уровня
    if (!currentFilter.level.empty() && entry.level != currentFilter.level) {
        return false;
    }
    
    // Проверка Event ID
    if (currentFilter.eventId != 0 && entry.eventId != currentFilter.eventId) {
        return false;
    }
    
    // Проверка ключевых слов
    if (!currentFilter.keyword.empty()) {
        string lowerDesc = entry.description;
        string lowerKeyword = currentFilter.keyword;
        transform(lowerDesc.begin(), lowerDesc.end(), lowerDesc.begin(), ::tolower);
        transform(lowerKeyword.begin(), lowerKeyword.end(), lowerKeyword.begin(), ::tolower);
        
        if (lowerDesc.find(lowerKeyword) == string::npos) {
            return false;
        }
    }
    
    // Проверка типа события
    if (entry.level == "ERROR" && !currentFilter.showErrors) return false;
    if (entry.level == "WARNING" && !currentFilter.showWarnings) return false;
    if (entry.level == "INFO" && !currentFilter.showInfo) return false;
    
    return true;
}

void LogViewer::displayLogEntry(const LogEntry& entry) {
    cout << "[" << entry.timestamp << "] ";
    
    // Цветовая кодировка по уровню
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (entry.level == "ERROR") {
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
    } else if (entry.level == "WARNING") {
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    } else if (entry.level == "INFO") {
        SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    } else {
        SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    }
    
    cout << "[" << entry.level << "] ";
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    
    cout << "[" << entry.source << "] ";
    cout << "ID:" << entry.eventId << " ";
    cout << entry.description << endl;
}

void LogViewer::showEntryDetails(const LogEntry& entry) {
    system("cls");
    cout << "========================================" << endl;
    cout << "              LOG ENTRY DETAILS" << endl;
    cout << "========================================" << endl << endl;

    cout << "Timestamp:   " << entry.timestamp << endl;
    cout << "Level:       " << entry.level << endl;
    cout << "Source:      " << entry.source << endl;
    cout << "Category:    " << entry.category << endl;
    cout << "Event ID:    " << entry.eventId << endl;
    cout << "Computer:    " << entry.computer << endl;
    cout << endl;
    cout << "Description:" << endl;
    cout << entry.description << endl;

    cout << endl << "Press any key to go back...";
    _getch();
}

void LogViewer::viewWindowsEventLogs() {
    system("cls");
    cout << "========================================" << endl;
    cout << "    WINDOWS EVENT LOGS" << endl;
    cout << "========================================" << endl << endl;
    
    logEntries.clear();
    readEventLog("Application");
    
    cout << "Found " << logEntries.size() << " log entries" << endl << endl;
    
    int displayed = 0;
    int maxDisplay = 50; // Ограничиваем количество для производительности
    
    for (const auto& entry : logEntries) {
        if (matchesFilter(entry) && displayed < maxDisplay) {
            displayLogEntry(entry);
            displayed++;
        }
    }
    
    if (displayed >= maxDisplay) {
        cout << endl << "... and " << (logEntries.size() - displayed) << " more entries" << endl;
    }
    
    cout << endl << "Press any key to continue...";
    _getch();
}

void LogViewer::viewApplicationLogs() {
    system("cls");
    cout << "========================================" << endl;
    cout << "    APPLICATION LOGS" << endl;
    cout << "========================================" << endl << endl;
    
    logEntries.clear();
    readEventLog("Application");
    
    cout << "Found " << logEntries.size() << " application log entries" << endl << endl;
    
    int displayed = 0;
    for (const auto& entry : logEntries) {
        if (matchesFilter(entry) && displayed < 30) {
            displayLogEntry(entry);
            displayed++;
        }
    }
    
    cout << endl << "Press any key to continue...";
    _getch();
}

void LogViewer::viewSecurityLogs() {
    system("cls");
    cout << "========================================" << endl;
    cout << "    SECURITY LOGS" << endl;
    cout << "========================================" << endl << endl;
    
    logEntries.clear();
    readEventLog("Security");
    
    cout << "Found " << logEntries.size() << " security log entries" << endl << endl;
    
    int displayed = 0;
    for (const auto& entry : logEntries) {
        if (matchesFilter(entry) && displayed < 30) {
            displayLogEntry(entry);
            displayed++;
        }
    }
    
    cout << endl << "Press any key to continue...";
    _getch();
}

void LogViewer::viewSystemLogs() {
    system("cls");
    cout << "========================================" << endl;
    cout << "    SYSTEM LOGS" << endl;
    cout << "========================================" << endl << endl;
    
    logEntries.clear();
    readEventLog("System");
    
    cout << "Found " << logEntries.size() << " system log entries" << endl << endl;
    
    int displayed = 0;
    for (const auto& entry : logEntries) {
        if (matchesFilter(entry) && displayed < 30) {
            displayLogEntry(entry);
            displayed++;
        }
    }
    
    cout << endl << "Press any key to continue...";
    _getch();
}

void LogViewer::searchLogs() {
    system("cls");
    cout << "========================================" << endl;
    cout << "    LOG SEARCH" << endl;
    cout << "========================================" << endl << endl;
    
    cout << "Enter search keyword: ";
    string keyword;
    getline(cin, keyword);
    
    if (keyword.empty()) {
        cout << "No keyword entered." << endl;
        cout << "Press any key to continue...";
        _getch();
        return;
    }
    
    currentFilter.keyword = keyword;
    
    cout << "Searching in all logs..." << endl << endl;
    
    // Поиск во всех типах логов
    vector<string> logTypes = {"Application", "System", "Security"};
    int totalFound = 0;
    
    for (const auto& logType : logTypes) {
        logEntries.clear();
        readEventLog(logType);
        
        int found = 0;
        for (const auto& entry : logEntries) {
            if (matchesFilter(entry)) {
                displayLogEntry(entry);
                found++;
                totalFound++;
            }
        }
        
        if (found > 0) {
            cout << endl << "Found " << found << " entries in " << logType << " log" << endl;
        }
    }
    
    cout << endl << "Total found: " << totalFound << " entries" << endl;
    cout << "Press any key to continue...";
    _getch();
}

void LogViewer::browseLogs(const string& logName) {
    logEntries.clear();
    readEventLog(logName);

    int selected = 0;
    int top = 0;
    int viewport = 20;
    bool running = true;

    while (running) {
        system("cls");
        cout << "========================================" << endl;
        cout << "           " << logName << " LOGS (" << logEntries.size() << ")" << endl;
        cout << "========================================" << endl << endl;

        int shown = 0;
        for (size_t i = 0, idx = 0; i < logEntries.size(); ++i) {
            if (!matchesFilter(logEntries[i])) continue;
            if ((int)idx < top) { idx++; continue; }
            bool isSelected = ((int)idx == selected);
            HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
            if (isSelected) SetConsoleTextAttribute(hConsole, BACKGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
            cout << (isSelected ? "> " : "  ");
            cout << "[" << logEntries[i].timestamp << "] [" << logEntries[i].level << "] ";
            cout << logEntries[i].source << " (#" << logEntries[i].eventId << ")" << endl;
            if (isSelected) SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            shown++; idx++;
            if (shown >= viewport) break;
        }

        cout << endl;
        cout << "Use ↑/↓ to scroll, Enter to view details, F to filter, Esc to back" << endl;

        int ch = _getch();
        if (ch == 27) break; // Esc
        if (ch == 13) {
            // Open selected entry
            int idx = -1; int filteredIndex = -1;
            for (size_t i = 0; i < logEntries.size(); ++i) {
                if (!matchesFilter(logEntries[i])) continue;
                filteredIndex++;
                if (filteredIndex == selected) { idx = (int)i; break; }
            }
            if (idx >= 0) showEntryDetails(logEntries[idx]);
            continue;
        }
        if (ch == 'f' || ch == 'F') { showLogFilterMenu(); top = 0; selected = 0; continue; }
        if (ch == 224 || ch == 0) {
            int code = _getch();
            if (code == 72) { // Up
                if (selected > 0) { selected--; if (selected < top) top = selected; }
            } else if (code == 80) { // Down
                // count filtered entries
                int count = 0; for (const auto &e : logEntries) if (matchesFilter(e)) count++;
                if (selected < count - 1) { selected++; if (selected >= top + viewport) top = selected - viewport + 1; }
            }
        }
    }
}

void LogViewer::showLogViewerMenu() {
    bool running = true;
    
    while (running) {
        system("cls");
        cout << "========================================" << endl;
        cout << "    LOG VIEWER" << endl;
        cout << "========================================" << endl << endl;
        
        cout << "Choose an option:" << endl << endl;
        cout << "[1] Windows Event Logs" << endl;
        cout << "[2] Application Logs" << endl;
        cout << "[3] Security Logs" << endl;
        cout << "[4] System Logs" << endl;
        cout << "[5] Search Logs" << endl;
        cout << "[6] Set Filter" << endl;
        cout << "[7] Export Logs" << endl;
        cout << "[8] Log Statistics" << endl;
        cout << "[9] Back to main menu" << endl << endl;
        
        cout << "Enter your choice (1-9): ";
        
        char choice = _getch();
        cout << endl << endl;
        
        switch (choice) {
            case '1':
                viewWindowsEventLogs();
                break;
            case '2':
                viewApplicationLogs();
                break;
            case '3':
                viewSecurityLogs();
                break;
            case '4':
                viewSystemLogs();
                break;
            case '5':
                searchLogs();
                break;
            case '6':
                showLogFilterMenu();
                break;
            case '7': {
                cout << "Enter export filename: ";
                string filename;
                getline(cin, filename);
                if (!filename.empty()) {
                    exportLogs(filename);
                }
                break;
            }
            case '8':
                showLogStatistics();
                break;
            case '9':
                running = false;
                break;
            default:
                cout << "Invalid choice!" << endl;
                break;
        }
        
        if (running) {
            cout << endl << "Press any key to continue...";
            _getch();
        }
    }
}

void LogViewer::showLogFilterMenu() {
    system("cls");
    cout << "========================================" << endl;
    cout << "    LOG FILTER SETTINGS" << endl;
    cout << "========================================" << endl << endl;
    
    cout << "Current filter settings:" << endl;
    cout << "Show Errors: " << (currentFilter.showErrors ? "Yes" : "No") << endl;
    cout << "Show Warnings: " << (currentFilter.showWarnings ? "Yes" : "No") << endl;
    cout << "Show Info: " << (currentFilter.showInfo ? "Yes" : "No") << endl;
    cout << "Source: " << (currentFilter.source.empty() ? "All" : currentFilter.source) << endl;
    cout << "Level: " << (currentFilter.level.empty() ? "All" : currentFilter.level) << endl;
    cout << "Event ID: " << (currentFilter.eventId == 0 ? "All" : to_string(currentFilter.eventId)) << endl;
    cout << "Keyword: " << (currentFilter.keyword.empty() ? "None" : currentFilter.keyword) << endl << endl;
    
    cout << "Options:" << endl;
    cout << "[1] Toggle Errors" << endl;
    cout << "[2] Toggle Warnings" << endl;
    cout << "[3] Toggle Info" << endl;
    cout << "[4] Set Source Filter" << endl;
    cout << "[5] Set Level Filter" << endl;
    cout << "[6] Set Event ID Filter" << endl;
    cout << "[7] Set Keyword Filter" << endl;
    cout << "[8] Clear All Filters" << endl;
    cout << "[9] Back to Log Viewer" << endl << endl;
    
    cout << "Enter your choice (1-9): ";
    
    char choice = _getch();
    cout << endl << endl;
    
    switch (choice) {
        case '1':
            currentFilter.showErrors = !currentFilter.showErrors;
            break;
        case '2':
            currentFilter.showWarnings = !currentFilter.showWarnings;
            break;
        case '3':
            currentFilter.showInfo = !currentFilter.showInfo;
            break;
        case '4': {
            cout << "Enter source filter: ";
            getline(cin, currentFilter.source);
            break;
        }
        case '5': {
            cout << "Enter level filter (ERROR/WARNING/INFO): ";
            getline(cin, currentFilter.level);
            break;
        }
        case '6': {
            cout << "Enter Event ID (0 for all): ";
            cin >> currentFilter.eventId;
            break;
        }
        case '7': {
            cout << "Enter keyword: ";
            getline(cin, currentFilter.keyword);
            break;
        }
        case '8':
            clearFilter();
            break;
        case '9':
            return;
        default:
            cout << "Invalid choice!" << endl;
            break;
    }
    
    cout << "Filter updated. Press any key to continue...";
    _getch();
}

void LogViewer::exportLogs(const string& filename) {
    ofstream file(filename);
    if (!file) {
        cout << "Error: Cannot create file " << filename << endl;
        return;
    }
    
    file << "Windows Event Log Export" << endl;
    file << "========================" << endl << endl;
    
    for (const auto& entry : logEntries) {
        if (matchesFilter(entry)) {
            file << "[" << entry.timestamp << "] ";
            file << "[" << entry.level << "] ";
            file << "[" << entry.source << "] ";
            file << "ID:" << entry.eventId << " ";
            file << entry.description << endl;
        }
    }
    
    file.close();
    cout << "Logs exported to " << filename << endl;
}

void LogViewer::showLogStatistics() {
    system("cls");
    cout << "========================================" << endl;
    cout << "    LOG STATISTICS" << endl;
    cout << "========================================" << endl << endl;
    
    int errorCount = 0, warningCount = 0, infoCount = 0;
    map<string, int> sourceCount;
    
    for (const auto& entry : logEntries) {
        if (entry.level == "ERROR") errorCount++;
        else if (entry.level == "WARNING") warningCount++;
        else if (entry.level == "INFO") infoCount++;
        
        sourceCount[entry.source]++;
    }
    
    cout << "Total entries: " << logEntries.size() << endl;
    cout << "Errors: " << errorCount << endl;
    cout << "Warnings: " << warningCount << endl;
    cout << "Info: " << infoCount << endl << endl;
    
    cout << "Top sources:" << endl;
    int count = 0;
    for (const auto& source : sourceCount) {
        if (count < 10) {
            cout << source.first << ": " << source.second << " entries" << endl;
            count++;
        }
    }
    
    cout << endl << "Press any key to continue...";
    _getch();
}

void LogViewer::setFilter(const LogFilter& filter) {
    currentFilter = filter;
}

void LogViewer::applyFilter() {
    // Фильтр применяется автоматически при отображении
}

void LogViewer::clearFilter() {
    currentFilter = LogFilter();
    currentFilter.showErrors = true;
    currentFilter.showWarnings = true;
    currentFilter.showInfo = false;
}

// Глобальные функции
void viewWindowsLogs() {
    LogViewer viewer;
    viewer.viewWindowsEventLogs();
}

void searchWindowsLogs(const string& keyword) {
    LogViewer viewer;
    LogFilter filter;
    filter.keyword = keyword;
    viewer.setFilter(filter);
    viewer.searchLogs();
}

void exportWindowsLogs(const string& logName, const string& filename) {
    LogViewer viewer;
    viewer.readEventLog(logName);
    viewer.exportLogs(filename);
}

vector<LogEntry> getRecentErrors(int count) {
    LogViewer viewer;
    LogFilter filter;
    filter.showErrors = true;
    filter.showWarnings = false;
    filter.showInfo = false;
    viewer.setFilter(filter);
    
    vector<LogEntry> errors;
    for (const auto& entry : viewer.logEntries) {
        if (viewer.matchesFilter(entry) && errors.size() < count) {
            errors.push_back(entry);
        }
    }
    return errors;
}

vector<LogEntry> getSecurityEvents(int count) {
    LogViewer viewer;
    viewer.readEventLog("Security");
    
    vector<LogEntry> securityEvents;
    for (const auto& entry : viewer.logEntries) {
        if (securityEvents.size() < count) {
            securityEvents.push_back(entry);
        }
    }
    return securityEvents;
}
