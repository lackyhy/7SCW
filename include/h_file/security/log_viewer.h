#ifndef LOG_VIEWER_H
#define LOG_VIEWER_H

#include <windows.h>
#include <string>
#include <vector>
#include <map>

using namespace std;

struct LogEntry {
    string timestamp;
    string level;
    string source;
    string category;
    string description;
    DWORD eventId;
    string computer;
};

struct LogFilter {
    string source;
    string level;
    DWORD eventId;
    string dateFrom;
    string dateTo;
    string keyword;
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
    vector<LogEntry> logEntries;
    
    void readEventLog(const string& logName);
    bool matchesFilter(const LogEntry& entry);
    LogViewer();
    
    // Основные функции
    void viewWindowsEventLogs();
    void viewApplicationLogs();
    void viewSecurityLogs();
    void viewSystemLogs();
    void searchLogs();
    void browseLogs(const string& logName);
    
    // Фильтрация
    void setFilter(const LogFilter& filter);
    void applyFilter();
    void clearFilter();
    
    // Утилиты
    void exportLogs(const string& filename);
    void showLogStatistics();
    
    // Меню
    void showLogViewerMenu();
    void showLogFilterMenu();
};

// Функции для работы с логами
void viewWindowsLogs();
void searchWindowsLogs(const string& keyword);
void exportWindowsLogs(const string& logName, const string& filename);
vector<LogEntry> getRecentErrors(int count = 10);
vector<LogEntry> getSecurityEvents(int count = 10);

#endif // LOG_VIEWER_H
