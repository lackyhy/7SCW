#include "../h_file/argv.h"
#include "../h_file/main.h"
#include "../h_file/startup/restoreStartupSettings.h"
#include "../h_file/file_manager/file_manager.h"
#include "../h_file/file/clear_temp_file.h"
#include "../h_file/security/advanced_security_menu.h"

#include <iostream>


using namespace std;

BOOL isAdmin = FALSE;
BOOL safemode = FALSE;
BOOL clear_tempfile = FALSE;
BOOL clear_autorun = FALSE;
BOOL help = FALSE;
BOOL logs_menu = FALSE;
BOOL show_version = FALSE;

Argv globalArgv;

void Argv::set_arguments(int argc, char* argv[]) {
    arguments.clear();
    for (int i = 0; i < argc; ++i) {
        arguments.push_back(argv[i]);
    }
}

void Argv::argv_processing() {
    for (size_t i = 0; i < arguments.size(); ++i) {
        process_argc_(arguments[i]);
    }
//    process_argc();
}

void process_argc_(string name_argv) {
    if (name_argv == "--help" || name_argv == "-h") {
        help = TRUE;
    } else if (name_argv == "--version" || name_argv == "-v") {
        show_version = TRUE;
    } else if (name_argv == "-safemod" || name_argv == "-safemode") {
        safemode = TRUE;
    } else if (name_argv == "-clear_tempfile") {
        clear_tempfile = TRUE;
    } else if (name_argv == "-clear_autorun") {
        clear_autorun = TRUE;
    } else if (name_argv == "-logs_menu") {
        logs_menu = TRUE;
    }
//    process_argc();
}

void process_argc() {
    bool _exit_ = false;

    if (show_version) {
        cout << "Version: " << VERSION << endl;
        _exit_ = true;
    }

    if (help) {
        cout << "Command line arguments:" << endl;
        cout << "   -safemod/-safemode: run app with limited rights" << endl;
        cout << "   -clear_tempfile: call function clear_temp_file()" << endl;
        cout << "   -clear_autorun: call function restoreStartupSettings()" << endl;
        cout << "   --version/-v: show version" << endl;
        cout << "   --help/-h: show this help" << endl;
        cout << "   -logs: show logs menu" << endl << endl;
        _exit_ = true;
    }

    if (clear_tempfile) {
        clear_temp_file();
        _exit_ = true;
    }

    if (clear_autorun) {
        restoreStartupSettings();
        _exit_ = true;
    }

    if (logs_menu) {
        showAdvancedSecurityMenu();
        _exit_ = true;
    }

    if (_exit_) {
        exit(0);
    }
}

void process_command_line_args(int argc, char* argv[]) {
    globalArgv.set_arguments(argc, argv);
    globalArgv.argv_processing();
}