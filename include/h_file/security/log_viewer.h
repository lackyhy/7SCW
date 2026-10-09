#ifndef LOG_VIEWER_H
#define LOG_VIEWER_H

#include <windows.h>
#include <string>
#include <vector>
#include <map>

struct LogEntry {
    std::string timestamp;
    std::string level;
    std::string source;
    std::string category;
    std::string description;
    DWORD eventId;
    std::string computer;
};

struct LogFilter {
    std::string source;
    std::string level;
    DWORD eventId;
    std::string dateFrom;
    std::string dateTo;
    std::string keyword;
    bool showErrors;
    bool showWarnings;
    bool showInfo;
};

class LogViewer {
private:
    LogFilter currentFilter;
    
    void parseEventLogEntry(PEVENTLOGRECORD pRecord);
    void displayLogEntry(const LogEntry& entry);
    void showEntryDetails(const LogEntry& entry);
    
public:
    std::vector<LogEntry> logEntries;
    
    void readEventLog(const std::string& logName);
    bool matchesFilter(const LogEntry& entry);
    LogViewer();
    
    // Основные функции
    void viewWindowsEventLogs();
    void viewApplicationLogs();
    void viewSecurityLogs();
    void viewSystemLogs();
    void searchLogs();
    void browseLogs(const std::string& logName);
    
    // Фильтрация
    void setFilter(const LogFilter& filter);
    void applyFilter();
    void clearFilter();
    
    // Утилиты
    void exportLogs(const std::string& filename);
    void showLogStatistics();
    
    // Меню
    void showLogViewerMenu();
    void showLogFilterMenu();
};

// Функции для работы с логами
void viewWindowsLogs();
void searchWindowsLogs(const std::string& keyword);
void exportWindowsLogs(const std::string& logName, const std::string& filename);
std::vector<LogEntry> getRecentErrors(int count = 10);
std::vector<LogEntry> getSecurityEvents(int count = 10);

#endif // LOG_VIEWER_H
