#include "../h_file/argv.h"
#include "../h_file/main.h"
#include "../h_file/startup/restoreStartupSettings.h"
#include "../h_file/file_manager/file_manager.h"
#include "../h_file/file/clear_temp_file.h"
#include "../h_file/security/advanced_security_menu.h"
#include "../h_file/system_info/system_info.h"
#include "../h_file/terminal/terminal.h"


#include <iostream>


using namespace std;

BOOL isAdmin = FALSE;
BOOL safemode = FALSE;
BOOL clear_tempfile = FALSE;
BOOL clear_autorun = FALSE;
BOOL help = FALSE;
BOOL logs_menu = FALSE;
BOOL logs_enabled = FALSE;
BOOL logs_console = FALSE;
BOOL show_version = FALSE;
BOOL system_menu = FALSE;
BOOL terminal_ = FALSE;
BOOL clear_logs = FALSE;


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
    if (name_argv == "--help" || name_argv == "-h" || name_argv == "-help") {
        help = TRUE;
    } else if (name_argv == "--version" || name_argv == "-v" || name_argv == "-version") {
        show_version = TRUE;
    } else if (name_argv == "-safemod" || name_argv == "-safemode" || name_argv == "--safemode" || name_argv == "--safemod") {
        safemode = TRUE;
    } else if (name_argv == "-clear_tempfile" || name_argv == "--clear_tempfile") {
        clear_tempfile = TRUE;
    } else if (name_argv == "-clear_autorun" || name_argv == "--clear_autorun") {
        clear_autorun = TRUE;
    } else if (name_argv == "-logs_menu" || name_argv == "--logs_menu" ) {
        logs_menu = TRUE;
    } else if (name_argv == "--logs" || name_argv == "-logs") {
        logs_enabled = TRUE;
    } else if (name_argv == "--logs_console" || name_argv == "-logs_console") {
        logs_console = TRUE;
    } else if (name_argv == "--systemInfo" || name_argv == "-systemInfo") {
        system_menu = TRUE;
    } else if (name_argv == "-terminal" || name_argv == "--terminal") {
        terminal_ = TRUE;
    } else if (name_argv == "-clear_logs" || name_argv == "--clear_logs") {
        clear_logs = TRUE;
    }
}

void process_argc() {
    bool _exit_ = false;

    if (clear_logs) {
        string logs_fileName = "logs.txt";
        system(("del /f /a:h " + logs_fileName).c_str());
    }

    if (terminal_) {
        customTerminal();
    }

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
        cout << "   -logs_menu: show logs menu" << endl;
        cout << "   --logs: enable logging to logs.txt file" << endl;
        cout << "   -logs_console: open separate console window for logs" << endl;
        cout << "   -||--systemInfo - print System Info" << endl;
        cout << "   -||--terminal - start terminal" << endl;
        cout << "   -||--clear_logs - clear logs and deleted file 'logs.txt'" << endl << endl;
        _exit_ = true;
    }
    
    if (system_menu) {
        print_SystemInfo();
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
        _exit_ = false;
    }

    if (_exit_) {
        exit(0);
    }
}

void process_command_line_args(int argc, char* argv[]) {
    globalArgv.set_arguments(argc, argv);
    globalArgv.argv_processing();
}