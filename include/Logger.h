#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <windows.h>

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
    static std::string log_file_path;
    static HANDLE console_handle;
    static HWND console_window;
    
public:
    static void initialize(bool enable_logging = false, bool enable_console = false, const std::string& file_path = "logs.txt");
    
    static void functions_log(LogLevel level, const std::string& funct, const std::string& message);
    static void log(LogLevel level, const std::string& message);
    
    static void info(const std::string& message);
    static void warning(const std::string& message);
    static void error(const std::string& message);
    static void success(const std::string& message);

    static std::string getCurrentTime();
    
    static bool isLoggingEnabled();
    static bool isConsoleLoggingEnabled();
    
    static void openLogFile();
    
    static void createLogConsole();
    
    static void closeLogConsole();
};

void printMessage(const std::string& message, bool isError = false);
void printWarning(const std::string& message);
void printInfo(const std::string& message);
void printError(const std::string& message);

#endif
