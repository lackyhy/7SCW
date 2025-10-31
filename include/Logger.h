#ifndef LOGGER_H
#define LOGGER_H

#include "string"
#include <fstream>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <windows.h>

using namespace std;

enum LogLevel {
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
    LOG_SUCCESS
};

class Logger {
private:
    static bool logging_enabled;
    static bool console_logging_enabled;
    static string log_file_path;
    static HANDLE console_handle;
    static HWND console_window;
    
public:
    static void initialize(bool enable_logging = false, bool enable_console = false, const string& file_path = "logs.txt");
    
    static void functions_log(LogLevel level, const string& funct, const string& message);
    static void log(LogLevel level, const string& message);
    
    static void info(const string& message);
    static void warning(const string& message);
    static void error(const string& message);
    static void success(const string& message);

    static string getCurrentTime();
    
    static bool isLoggingEnabled();
    static bool isConsoleLoggingEnabled();
    
    static void openLogFile();
    
    static void createLogConsole();
    
    static void closeLogConsole();
};

void printMessage(const string& message, bool isError = false);
void printWarning(const string& message);
void printInfo(const string& message);
void printError(const string& message);

#endif
