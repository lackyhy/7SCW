#ifndef LOGS_LOGS
#define LOGS_LOGS
#include "string"
#include <fstream>
#include <ctime>
#include <iomanip>
#include <sstream>

using namespace std;

// Enum для уровней логирования
enum LogLevel {
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
    LOG_SUCCESS
};

// Класс для работы с логами
class Logger {
private:
    static bool logging_enabled;
    static string log_file_path;
    
public:
    // Инициализация логгера
    static void initialize(bool enable_logging = false, const string& file_path = "logs.txt");
    
    // Основная функция логирования
    static void log(LogLevel level, const string& message);
    
    // Удобные функции для разных уровней
    static void info(const string& message);
    static void warning(const string& message);
    static void error(const string& message);
    static void success(const string& message);
    
    // Получить текущее время в формате строки
    static string getCurrentTime();
    
    // Проверить, включено ли логирование
    static bool isLoggingEnabled();
    
    // Открыть файл логов
    static void openLogFile();
};

// Старые функции для совместимости
void printMessage(const string& message, bool isError = false);
void printWarning(const string& message);
void printInfo(const string& message);
void printError(const string& message);

#endif