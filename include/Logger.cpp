#include <iostream>
#include <string>
#include <windows.h>
#include <fstream>
#include <iomanip>
#include <cstring>
#include <sstream>
#include <cstdio>
#include <conio.h>

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

// Статические переменные класса Logger
bool Logger::logging_enabled = false;
bool Logger::console_logging_enabled = false;
string Logger::log_file_path = "logs.txt";
HANDLE Logger::console_handle = nullptr;
HWND Logger::console_window = nullptr;

// Инициализация логгера
void Logger::initialize(bool enable_logging, bool enable_console, const string& file_path) {
    Logger::info("Initializing logger - File logging: " + string(enable_logging ? "true" : "false") + 
                 ", Console logging: " + string(enable_console ? "true" : "false") + 
                 ", Log file: " + file_path);
    
    logging_enabled = enable_logging;
    console_logging_enabled = enable_console;
    log_file_path = file_path;
    
    if (console_logging_enabled) {
        Logger::info("Console logging enabled - creating separate console");
        createLogConsole();
    }
    
    if (logging_enabled) {
        Logger::info("File logging enabled - creating log file");
        // Создаем файл логов и записываем заголовок
        ofstream log_file(log_file_path, ios::app);
        if (log_file.is_open()) {
            log_file << "\n=== LOG SESSION STARTED: " << getCurrentTime() << " ===" << endl;
            log_file.close();
            Logger::success("Log file created successfully: " + log_file_path);
            
            // Делаем файл скрытым
            #ifdef _WIN32
            if (SetFileAttributesA(log_file_path.c_str(), FILE_ATTRIBUTE_HIDDEN)) {
                Logger::info("Log file set as hidden");
            } else {
                Logger::warning("Failed to set log file as hidden");
            }
            #endif
        } else {
            Logger::error("Failed to create log file: " + log_file_path);
        }
    }
    
    Logger::info("Logger initialization completed");
}

void Logger::functions_log(LogLevel level, const string& funct, const string& message) {
    string level_str;
    string color_code;
    string _message;

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
    _message = "FUNCT: " + funct + "  " + message;
    log(level, _message);
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
    
    // Выводим в основную консоль с цветом
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
    
    // Выводим в отдельную консоль логов, если она включена
    if (console_logging_enabled && console_handle != nullptr) {
        // Проверяем, что консоль все еще доступна
        DWORD consoleMode;
        if (GetConsoleMode(console_handle, &consoleMode)) {
            SetConsoleTextAttribute(console_handle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            string log_message = "[" + getCurrentTime() + "] " + level_str + " " + message;
            
            DWORD written;
            WriteConsoleA(console_handle, log_message.c_str(), log_message.length(), &written, nullptr);
            WriteConsoleA(console_handle, "\n", 1, &written, nullptr);
        } else {
            // Консоль была закрыта, сбрасываем флаг
            console_logging_enabled = false;
            console_handle = nullptr;
            console_window = nullptr;
        }
    }
    
    // Записываем в файл, если логирование включено или включена консоль логов
    if (logging_enabled || console_logging_enabled) {
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

bool Logger::isConsoleLoggingEnabled() {
    return console_logging_enabled;
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

// Создать отдельную консоль для логов
void Logger::createLogConsole() {
    Logger::info("Starting console creation process");
    
    // Создаем новую консоль напрямую
    if (AllocConsole()) {
        Logger::success("Console allocated successfully");
        
        // Получаем handle новой консоли
        console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
        Logger::info("Console handle obtained: " + to_string((long long)console_handle));
        
        // Получаем handle окна консоли
        console_window = GetConsoleWindow();
        Logger::info("Console window handle obtained: " + to_string((long long)console_window));
        
        // Устанавливаем заголовок окна
        if (SetConsoleTitleA("7SCW Logs Console")) {
            Logger::success("Console title set successfully");
        } else {
            DWORD error = GetLastError();
            Logger::warning("Failed to set console title. Error code: " + to_string(error));
        }
        
        // Устанавливаем размер окна
        COORD bufferSize = {120, 30};
        if (SetConsoleScreenBufferSize(console_handle, bufferSize)) {
            Logger::success("Console buffer size set to 120x30");
        } else {
            DWORD error = GetLastError();
            Logger::warning("Failed to set console buffer size. Error code: " + to_string(error));
        }
        
        // Позиционируем окно
        if (SetWindowPos(console_window, HWND_TOP, 100, 100, 800, 600, SWP_SHOWWINDOW)) {
            Logger::success("Console window positioned at (100, 100) with size 800x600");
        } else {
            DWORD error = GetLastError();
            Logger::warning("Failed to position console window. Error code: " + to_string(error));
        }
        
        // Выводим заголовок
        SetConsoleTextAttribute(console_handle, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        string header = "=== 7SCW LOGS CONSOLE - Session started at " + getCurrentTime() + " ===\n";
        DWORD written;
        if (WriteConsoleA(console_handle, header.c_str(), header.length(), &written, nullptr)) {
            Logger::success("Header written to console successfully");
        } else {
            DWORD error = GetLastError();
            Logger::warning("Failed to write header to console. Error code: " + to_string(error));
        }
        WriteConsoleA(console_handle, "\n", 1, &written, nullptr);
        
        // Сбрасываем цвет
        SetConsoleTextAttribute(console_handle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        
        // НЕ освобождаем консоль - она должна остаться открытой
        Logger::success("Console creation completed successfully");
    } else {
        DWORD error = GetLastError();
        Logger::error("Failed to allocate console. Error code: " + to_string(error));
    }
    
    Logger::info("Console creation process completed");
}

// Закрыть консоль логов
void Logger::closeLogConsole() {
    Logger::info("Attempting to close log console");
    
    if (console_handle != nullptr) {
        Logger::info("Console handle found - writing closing message");
        // Выводим сообщение о завершении сессии
        SetConsoleTextAttribute(console_handle, FOREGROUND_RED | FOREGROUND_INTENSITY);
        string footer = "\n=== LOG SESSION ENDED at " + getCurrentTime() + " ===\n";
        DWORD written;
        WriteConsoleA(console_handle, footer.c_str(), footer.length(), &written, nullptr);
        WriteConsoleA(console_handle, "Press any key to close this window...", 35, &written, nullptr);
        
        // Ждем нажатия клавиши
        _getch();
        
        // Закрываем консоль
        if (FreeConsole()) {
            Logger::success("Console freed successfully");
        } else {
            DWORD error = GetLastError();
            Logger::warning("Failed to free console. Error code: " + to_string(error));
        }
        console_handle = nullptr;
        console_window = nullptr;
        Logger::info("Console handles reset to nullptr");
    } else {
        Logger::warning("Console handle is nullptr - nothing to close");
    }
    
    Logger::info("Log console close process completed");
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