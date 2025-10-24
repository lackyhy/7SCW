#include <iostream>
#include <string>
#include <windows.h>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <sstream>
#include "../../h_file/logs/logs.h"
#include "../../h_file/argv.h"

using namespace std;

// Статические переменные класса Logger
bool Logger::logging_enabled = false;
string Logger::log_file_path = "logs.txt";

// Инициализация логгера
void Logger::initialize(bool enable_logging, const string& file_path) {
    logging_enabled = enable_logging;
    log_file_path = file_path;
    
    if (logging_enabled) {
        // Создаем файл логов и записываем заголовок
        ofstream log_file(log_file_path, ios::app);
        if (log_file.is_open()) {
            log_file << "\n=== LOG SESSION STARTED: " << getCurrentTime() << " ===" << endl;
            log_file.close();
            
            // Делаем файл скрытым
            #ifdef _WIN32
            SetFileAttributesA(log_file_path.c_str(), FILE_ATTRIBUTE_HIDDEN);
            #endif
        }
    }
}

// Основная функция логирования
void Logger::log(LogLevel level, const string& message) {
    string level_str;
    string color_code;
    
    // Определяем уровень и цвет
    switch (level) {
        case LOG_INFO:
            level_str = "[INFO]";
            color_code = "BLUE";
            break;
        case LOG_WARNING:
            level_str = "[WARNING]";
            color_code = "YELLOW";
            break;
        case LOG_ERROR:
            level_str = "[ERROR]";
            color_code = "RED";
            break;
        case LOG_SUCCESS:
            level_str = "[SUCCESS]";
            color_code = "GREEN";
            break;
    }
    
    // Выводим в консоль с цветом
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    switch (level) {
        case LOG_INFO:
            SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
            break;
        case LOG_WARNING:
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
            break;
        case LOG_ERROR:
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
            break;
        case LOG_SUCCESS:
            SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
            break;
    }
    
    cout << level_str << " ";
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    cout << message << endl;
    
    // Записываем в файл, если логирование включено
    if (logging_enabled) {
        ofstream log_file(log_file_path, ios::app);
        if (log_file.is_open()) {
            log_file << "[" << getCurrentTime() << "] " << level_str << " " << message << endl;
            log_file.close();
        }
    }
}

// Удобные функции для разных уровней
void Logger::info(const string& message) {
    log(LOG_INFO, message);
}

void Logger::warning(const string& message) {
    log(LOG_WARNING, message);
}

void Logger::error(const string& message) {
    log(LOG_ERROR, message);
}

void Logger::success(const string& message) {
    log(LOG_SUCCESS, message);
}

// Получить текущее время в формате строки
string Logger::getCurrentTime() {
    time_t now = time(0);
    struct tm* timeinfo = localtime(&now);
    
    stringstream ss;
    ss << put_time(timeinfo, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

// Проверить, включено ли логирование
bool Logger::isLoggingEnabled() {
    return logging_enabled;
}

// Открыть файл логов
void Logger::openLogFile() {
    if (logging_enabled) {
        string command = "notepad " + log_file_path;
        system(command.c_str());
    } else {
        cout << "Logging is not enabled. Use --logs flag to enable logging." << endl;
    }
}

// Старые функции для совместимости
void printMessage(const string& message, bool isError) {
    if (isError) {
        Logger::error(message);
    } else {
        Logger::success(message);
    }
}

void printWarning(const string& message) {
    Logger::warning(message);
}

void printInfo(const string& message) {
    Logger::info(message);
}

void printError(const string& message) {
    Logger::error(message);
}
