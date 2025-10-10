#ifndef ARGV_H
#define ARGV_H

#include <string>
#include <vector>
#include <windef.h>

using namespace std;

// Объявление глобальных переменных
extern BOOL isAdmin;
extern BOOL safemode;
extern BOOL clear_tempfile;
extern BOOL clear_autorun;
extern BOOL help;
extern BOOL logs_menu;
extern BOOL show_version;

extern const std::string VERSION;

class Argv {
private:
    std::vector<std::string> arguments;

public:
    // Устанавливает аргументы
    void set_arguments(int argc, char* argv[]);

    // Обрабатывает аргументы
    void argv_processing();
};

// Функции для обработки аргументов
void process_argc_(std::string name_argv);
void process_argc();
void process_command_line_args(int argc, char* argv[]);

#endif