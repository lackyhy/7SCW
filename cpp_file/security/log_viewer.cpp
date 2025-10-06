#include "../../h_file/security/log_viewer.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <conio.h>

LogViewer::LogViewer() {
    // Инициализация фильтра по умолчанию
    currentFilter.showErrors = true;
    currentFilter.showWarnings = true;
    currentFilter.showInfo = false;
}

void LogViewer::readEventLog(const std::string& logName) {
    HANDLE hEventLog = OpenEventLogA(NULL, logName.c_str());
    if (hEventLog == NULL) {
        std::cout << "Error: Cannot open event log " << logName << std::endl;
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
    
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(2) << st.wDay << "/"
       << std::setw(2) << st.wMonth << "/" << st.wYear << " "
       << std::setw(2) << st.wHour << ":" << std::setw(2) << st.wMinute << ":" << std::setw(2) << st.wSecond;
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
    if (!currentFilter.source.empty() && entry.source.find(currentFilter.source) == std::string::npos) {
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
        std::string lowerDesc = entry.description;
        std::string lowerKeyword = currentFilter.keyword;
        std::transform(lowerDesc.begin(), lowerDesc.end(), lowerDesc.begin(), ::tolower);
        std::transform(lowerKeyword.begin(), lowerKeyword.end(), lowerKeyword.begin(), ::tolower);
        
        if (lowerDesc.find(lowerKeyword) == std::string::npos) {
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
    std::cout << "[" << entry.timestamp << "] ";
    
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
    
    std::cout << "[" << entry.level << "] ";
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    
    std::cout << "[" << entry.source << "] ";
    std::cout << "ID:" << entry.eventId << " ";
    std::cout << entry.description << std::endl;
}

void LogViewer::showEntryDetails(const LogEntry& entry) {
    system("cls");
    std::cout << "========================================" << std::endl;
    std::cout << "              LOG ENTRY DETAILS" << std::endl;
    std::cout << "========================================" << std::endl << std::endl;

    std::cout << "Timestamp:   " << entry.timestamp << std::endl;
    std::cout << "Level:       " << entry.level << std::endl;
    std::cout << "Source:      " << entry.source << std::endl;
    std::cout << "Category:    " << entry.category << std::endl;
    std::cout << "Event ID:    " << entry.eventId << std::endl;
    std::cout << "Computer:    " << entry.computer << std::endl;
    std::cout << std::endl;
    std::cout << "Description:" << std::endl;
    std::cout << entry.description << std::endl;

    std::cout << std::endl << "Press any key to go back...";
    _getch();
}

void LogViewer::viewWindowsEventLogs() {
    system("cls");
    std::cout << "========================================" << std::endl;
    std::cout << "    WINDOWS EVENT LOGS" << std::endl;
    std::cout << "========================================" << std::endl << std::endl;
    
    logEntries.clear();
    readEventLog("Application");
    
    std::cout << "Found " << logEntries.size() << " log entries" << std::endl << std::endl;
    
    int displayed = 0;
    int maxDisplay = 50; // Ограничиваем количество для производительности
    
    for (const auto& entry : logEntries) {
        if (matchesFilter(entry) && displayed < maxDisplay) {
            displayLogEntry(entry);
            displayed++;
        }
    }
    
    if (displayed >= maxDisplay) {
        std::cout << std::endl << "... and " << (logEntries.size() - displayed) << " more entries" << std::endl;
    }
    
    std::cout << std::endl << "Press any key to continue...";
    _getch();
}

void LogViewer::viewApplicationLogs() {
    system("cls");
    std::cout << "========================================" << std::endl;
    std::cout << "    APPLICATION LOGS" << std::endl;
    std::cout << "========================================" << std::endl << std::endl;
    
    logEntries.clear();
    readEventLog("Application");
    
    std::cout << "Found " << logEntries.size() << " application log entries" << std::endl << std::endl;
    
    int displayed = 0;
    for (const auto& entry : logEntries) {
        if (matchesFilter(entry) && displayed < 30) {
            displayLogEntry(entry);
            displayed++;
        }
    }
    
    std::cout << std::endl << "Press any key to continue...";
    _getch();
}

void LogViewer::viewSecurityLogs() {
    system("cls");
    std::cout << "========================================" << std::endl;
    std::cout << "    SECURITY LOGS" << std::endl;
    std::cout << "========================================" << std::endl << std::endl;
    
    logEntries.clear();
    readEventLog("Security");
    
    std::cout << "Found " << logEntries.size() << " security log entries" << std::endl << std::endl;
    
    int displayed = 0;
    for (const auto& entry : logEntries) {
        if (matchesFilter(entry) && displayed < 30) {
            displayLogEntry(entry);
            displayed++;
        }
    }
    
    std::cout << std::endl << "Press any key to continue...";
    _getch();
}

void LogViewer::viewSystemLogs() {
    system("cls");
    std::cout << "========================================" << std::endl;
    std::cout << "    SYSTEM LOGS" << std::endl;
    std::cout << "========================================" << std::endl << std::endl;
    
    logEntries.clear();
    readEventLog("System");
    
    std::cout << "Found " << logEntries.size() << " system log entries" << std::endl << std::endl;
    
    int displayed = 0;
    for (const auto& entry : logEntries) {
        if (matchesFilter(entry) && displayed < 30) {
            displayLogEntry(entry);
            displayed++;
        }
    }
    
    std::cout << std::endl << "Press any key to continue...";
    _getch();
}

void LogViewer::searchLogs() {
    system("cls");
    std::cout << "========================================" << std::endl;
    std::cout << "    LOG SEARCH" << std::endl;
    std::cout << "========================================" << std::endl << std::endl;
    
    std::cout << "Enter search keyword: ";
    std::string keyword;
    std::getline(std::cin, keyword);
    
    if (keyword.empty()) {
        std::cout << "No keyword entered." << std::endl;
        std::cout << "Press any key to continue...";
        _getch();
        return;
    }
    
    currentFilter.keyword = keyword;
    
    std::cout << "Searching in all logs..." << std::endl << std::endl;
    
    // Поиск во всех типах логов
    std::vector<std::string> logTypes = {"Application", "System", "Security"};
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
            std::cout << std::endl << "Found " << found << " entries in " << logType << " log" << std::endl;
        }
    }
    
    std::cout << std::endl << "Total found: " << totalFound << " entries" << std::endl;
    std::cout << "Press any key to continue...";
    _getch();
}

void LogViewer::browseLogs(const std::string& logName) {
    logEntries.clear();
    readEventLog(logName);

    int selected = 0;
    int top = 0;
    int viewport = 20;
    bool running = true;

    while (running) {
        system("cls");
        std::cout << "========================================" << std::endl;
        std::cout << "           " << logName << " LOGS (" << logEntries.size() << ")" << std::endl;
        std::cout << "========================================" << std::endl << std::endl;

        int shown = 0;
        for (size_t i = 0, idx = 0; i < logEntries.size(); ++i) {
            if (!matchesFilter(logEntries[i])) continue;
            if ((int)idx < top) { idx++; continue; }
            bool isSelected = ((int)idx == selected);
            HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
            if (isSelected) SetConsoleTextAttribute(hConsole, BACKGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
            std::cout << (isSelected ? "> " : "  ");
            std::cout << "[" << logEntries[i].timestamp << "] [" << logEntries[i].level << "] ";
            std::cout << logEntries[i].source << " (#" << logEntries[i].eventId << ")" << std::endl;
            if (isSelected) SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            shown++; idx++;
            if (shown >= viewport) break;
        }

        std::cout << std::endl;
        std::cout << "Use ↑/↓ to scroll, Enter to view details, F to filter, Esc to back" << std::endl;

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
        std::cout << "========================================" << std::endl;
        std::cout << "    LOG VIEWER" << std::endl;
        std::cout << "========================================" << std::endl << std::endl;
        
        std::cout << "Choose an option:" << std::endl << std::endl;
        std::cout << "[1] Windows Event Logs" << std::endl;
        std::cout << "[2] Application Logs" << std::endl;
        std::cout << "[3] Security Logs" << std::endl;
        std::cout << "[4] System Logs" << std::endl;
        std::cout << "[5] Search Logs" << std::endl;
        std::cout << "[6] Set Filter" << std::endl;
        std::cout << "[7] Export Logs" << std::endl;
        std::cout << "[8] Log Statistics" << std::endl;
        std::cout << "[9] Back to main menu" << std::endl << std::endl;
        
        std::cout << "Enter your choice (1-9): ";
        
        char choice = _getch();
        std::cout << std::endl << std::endl;
        
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
                std::cout << "Enter export filename: ";
                std::string filename;
                std::getline(std::cin, filename);
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
                std::cout << "Invalid choice!" << std::endl;
                break;
        }
        
        if (running) {
            std::cout << std::endl << "Press any key to continue...";
            _getch();
        }
    }
}

void LogViewer::showLogFilterMenu() {
    system("cls");
    std::cout << "========================================" << std::endl;
    std::cout << "    LOG FILTER SETTINGS" << std::endl;
    std::cout << "========================================" << std::endl << std::endl;
    
    std::cout << "Current filter settings:" << std::endl;
    std::cout << "Show Errors: " << (currentFilter.showErrors ? "Yes" : "No") << std::endl;
    std::cout << "Show Warnings: " << (currentFilter.showWarnings ? "Yes" : "No") << std::endl;
    std::cout << "Show Info: " << (currentFilter.showInfo ? "Yes" : "No") << std::endl;
    std::cout << "Source: " << (currentFilter.source.empty() ? "All" : currentFilter.source) << std::endl;
    std::cout << "Level: " << (currentFilter.level.empty() ? "All" : currentFilter.level) << std::endl;
    std::cout << "Event ID: " << (currentFilter.eventId == 0 ? "All" : std::to_string(currentFilter.eventId)) << std::endl;
    std::cout << "Keyword: " << (currentFilter.keyword.empty() ? "None" : currentFilter.keyword) << std::endl << std::endl;
    
    std::cout << "Options:" << std::endl;
    std::cout << "[1] Toggle Errors" << std::endl;
    std::cout << "[2] Toggle Warnings" << std::endl;
    std::cout << "[3] Toggle Info" << std::endl;
    std::cout << "[4] Set Source Filter" << std::endl;
    std::cout << "[5] Set Level Filter" << std::endl;
    std::cout << "[6] Set Event ID Filter" << std::endl;
    std::cout << "[7] Set Keyword Filter" << std::endl;
    std::cout << "[8] Clear All Filters" << std::endl;
    std::cout << "[9] Back to Log Viewer" << std::endl << std::endl;
    
    std::cout << "Enter your choice (1-9): ";
    
    char choice = _getch();
    std::cout << std::endl << std::endl;
    
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
            std::cout << "Enter source filter: ";
            std::getline(std::cin, currentFilter.source);
            break;
        }
        case '5': {
            std::cout << "Enter level filter (ERROR/WARNING/INFO): ";
            std::getline(std::cin, currentFilter.level);
            break;
        }
        case '6': {
            std::cout << "Enter Event ID (0 for all): ";
            std::cin >> currentFilter.eventId;
            break;
        }
        case '7': {
            std::cout << "Enter keyword: ";
            std::getline(std::cin, currentFilter.keyword);
            break;
        }
        case '8':
            clearFilter();
            break;
        case '9':
            return;
        default:
            std::cout << "Invalid choice!" << std::endl;
            break;
    }
    
    std::cout << "Filter updated. Press any key to continue...";
    _getch();
}

void LogViewer::exportLogs(const std::string& filename) {
    std::ofstream file(filename);
    if (!file) {
        std::cout << "Error: Cannot create file " << filename << std::endl;
        return;
    }
    
    file << "Windows Event Log Export" << std::endl;
    file << "========================" << std::endl << std::endl;
    
    for (const auto& entry : logEntries) {
        if (matchesFilter(entry)) {
            file << "[" << entry.timestamp << "] ";
            file << "[" << entry.level << "] ";
            file << "[" << entry.source << "] ";
            file << "ID:" << entry.eventId << " ";
            file << entry.description << std::endl;
        }
    }
    
    file.close();
    std::cout << "Logs exported to " << filename << std::endl;
}

void LogViewer::showLogStatistics() {
    system("cls");
    std::cout << "========================================" << std::endl;
    std::cout << "    LOG STATISTICS" << std::endl;
    std::cout << "========================================" << std::endl << std::endl;
    
    int errorCount = 0, warningCount = 0, infoCount = 0;
    std::map<std::string, int> sourceCount;
    
    for (const auto& entry : logEntries) {
        if (entry.level == "ERROR") errorCount++;
        else if (entry.level == "WARNING") warningCount++;
        else if (entry.level == "INFO") infoCount++;
        
        sourceCount[entry.source]++;
    }
    
    std::cout << "Total entries: " << logEntries.size() << std::endl;
    std::cout << "Errors: " << errorCount << std::endl;
    std::cout << "Warnings: " << warningCount << std::endl;
    std::cout << "Info: " << infoCount << std::endl << std::endl;
    
    std::cout << "Top sources:" << std::endl;
    int count = 0;
    for (const auto& source : sourceCount) {
        if (count < 10) {
            std::cout << source.first << ": " << source.second << " entries" << std::endl;
            count++;
        }
    }
    
    std::cout << std::endl << "Press any key to continue...";
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

void searchWindowsLogs(const std::string& keyword) {
    LogViewer viewer;
    LogFilter filter;
    filter.keyword = keyword;
    viewer.setFilter(filter);
    viewer.searchLogs();
}

void exportWindowsLogs(const std::string& logName, const std::string& filename) {
    LogViewer viewer;
    viewer.readEventLog(logName);
    viewer.exportLogs(filename);
}

std::vector<LogEntry> getRecentErrors(int count) {
    LogViewer viewer;
    LogFilter filter;
    filter.showErrors = true;
    filter.showWarnings = false;
    filter.showInfo = false;
    viewer.setFilter(filter);
    
    std::vector<LogEntry> errors;
    for (const auto& entry : viewer.logEntries) {
        if (viewer.matchesFilter(entry) && errors.size() < count) {
            errors.push_back(entry);
        }
    }
    return errors;
}

std::vector<LogEntry> getSecurityEvents(int count) {
    LogViewer viewer;
    viewer.readEventLog("Security");
    
    std::vector<LogEntry> securityEvents;
    for (const auto& entry : viewer.logEntries) {
        if (securityEvents.size() < count) {
            securityEvents.push_back(entry);
        }
    }
    return securityEvents;
}
