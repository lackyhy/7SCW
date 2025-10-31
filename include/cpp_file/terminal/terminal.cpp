#include "iostream"
#include "string"
#include "vector"
#include "regex"
#include "conio.h"
#include "fstream"

#include "windows.h"
#include "../../h_file/terminal/terminal_commands.h"
#include "../../h_file/terminal/terminal.h"
#include "../../h_file/activator/menu.h"
#include "../../h_file/file_manager/file_manager.h"
#include "../../h_file/file/clear_cookie.h"
#include "../../Logger.h"
#include "../../h_file/main.h"

using namespace std;

// List of commands for auto-completion
vector<string> availableCommands = {
    "activate_", "update_programm", "help", "exit", "quit", "q",
    "ls", "dir", "cd", "pwd", "mkdir", "rm", "rmdir", "cp", "mv",
    "cat", "echo", "clear", "cls", "date", "whoami", "hostname",
    "systeminfo", "processes", "services", "netstat", "ipconfig",
    "encrypt", "deencrypt", "create_hash", "network", "search_file",
    "cookie_", "logs_"
};

// Function for command auto-completion
string autoCompleteCommand(const string& input) {
    vector<string> matches;
    
    // Find commands that start with the input text
    for (const auto& cmd : availableCommands) {
        if (cmd.find(input) == 0) { // starts with input
            matches.push_back(cmd);
        }
    }
    
    if (matches.empty()) {
        return input; // no matches found
    }
    else if (matches.size() == 1) {
        return matches[0]; // single match found
    }
    else {
        // Multiple matches - show options
        cout << "\nAvailable options:\n";
        for (const auto& match : matches) {
            cout << "  " << match << endl;
        }
        cout << "Enter more characters to narrow down..." << endl;
        
        // Return the shortest common prefix
        string commonPrefix = matches[0];
        for (size_t i = 1; i < matches.size(); i++) {
            size_t j = 0;
            while (j < commonPrefix.length() && j < matches[i].length() && 
                   commonPrefix[j] == matches[i][j]) {
                j++;
            }
            commonPrefix = commonPrefix.substr(0, j);
        }
        return commonPrefix;
    }
}

// Function to read input with Tab completion support
string readLineWithTabCompletion() {
    string input;
    char ch;
    
    while (true) {
        ch = _getch(); // read character without display
        
        if (ch == '\r' || ch == '\n') { // Enter
            cout << endl;
            return input;
        }
        else if (ch == '\t') { // Tab - auto-completion
            if (!input.empty()) {
                string completed = autoCompleteCommand(input);
                // Clear current line and show completed version
                for (size_t i = 0; i < input.length(); i++) {
                    cout << "\b \b"; // backspace
                }
                input = completed;
                cout << input;
            }
        }
        else if (ch == 8 || ch == 127) { // Backspace
            if (!input.empty()) {
                input.pop_back();
                cout << "\b \b"; // remove character from console
            }
        }
        else if (ch == 3) { // Ctrl+C
            cout << "^C" << endl;
            input = "";
            return input;
        }
        else if (isprint(ch)) { // Printable character
            input += ch;
            cout << ch;
        }
    }
}

void customTerminal() {
    system("cls");
    Logger::info("Opening custom terminal");
    cout << "=== Custom Terminal (Bash-like) ===" << endl;
    cout << "Type 'exit/q/quit' to return to main menu" << endl;
    cout << "Type 'help' for available commands and 'update_programm' for update programm" << endl;
    cout << "Type TAB for command auto-completion" << endl;
    cout << "===================================" << endl << endl;

    string currentDir = "C:\\";
    vector<string> commandHistory;
    int historyIndex = -1;

    while (true) {
        // Get current directory
        char buffer[MAX_PATH];
        GetCurrentDirectoryA(MAX_PATH, buffer);
        currentDir = string(buffer);

        // Display prompt
        cout << "[$" << currentDir << "] ";

        // Get user input with Tab completion
        string command = readLineWithTabCompletion();

        // Handle empty command
        if (command.empty()) {
            continue;
        }

        // Add to history
        commandHistory.push_back(command);
        historyIndex = commandHistory.size();

        // Parse command
        string cmd = command;
        transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);

        // Handle built-in commands
        if (cmd == "exit" || cmd == "quit" || cmd == "q") {
            Logger::info("Exiting custom terminal");
            break;
        }
        else if (cmd == "help") {
            Logger::info("Showing terminal help");
            cout << "\nAvailable commands:" << endl;
            cout << "  cookie_          - delete cookie yor browser" << endl;
            cout << "  logs_            - open logs.txt file" << endl;
            cout << "  activate_        - output of programs like KSM_AUTO" << endl;
            cout << "  update_programm  - check for program updates" << endl;
            cout << "  ls, dir          - List directory contents" << endl;
            cout << "  cd <path>        - Change directory" << endl;
            cout << "  pwd              - Print working directory" << endl;
            cout << "  mkdir <name>     - Create directory" << endl;
            cout << "  rm <file>        - Remove file" << endl;
            cout << "  rmdir <dir>      - Remove directory" << endl;
            cout << "  cp <src> <dst>   - Copy file" << endl;
            cout << "  mv <src> <dst>   - Move/rename file" << endl;
            cout << "  cat <file>       - Display file contents" << endl;
            cout << "  echo <text>      - Print text" << endl;
            cout << "  clear            - Clear screen" << endl;
            cout << "  date             - Show current date/time" << endl;
            cout << "  whoami           - Show current user" << endl;
            cout << "  hostname         - Show computer name" << endl;
            cout << "  systeminfo       - Show system information" << endl;
            cout << "  processes        - Show running processes" << endl;
            cout << "  services         - Show running services" << endl;
            cout << "  netstat          - Show network connections" << endl;
            cout << "  ipconfig         - Show network configuration" << endl;
            cout << "  encrypt -pass <pwd> <file> - Encrypt file with password" << endl;
            cout << "  deencrypt [file] - Decrypt file (will prompt for password)" << endl;
            cout << "  create_hash      - Generate random hashes with flags: -l, -q, -s, -f, -h" << endl;
            cout << "  network          - Network management and testing menu" << endl;
            cout << "  update_programm  - Check and update program from GitHub" << endl;
            cout << "  info             - Show program information and GitHub link" << endl;
            cout << "  help             - Show this help" << endl;
            cout << "  exit, quit       - Exit terminal" << endl;
            cout << "  Any other command will be executed as Windows command" << endl << endl;
        }
        else if (cmd == "info") {
            system("cls");
            cout << "=========================================" << endl;
            cout << "            PROGRAM INFORMATION" << endl;
            cout << "=========================================" << endl;
            cout << endl;
            cout << "Program Name: 7SCW (7 System Control Windows) - version " << VERSION << endl;
            cout << "Author: lackyhy (LCKY)" << endl;
            cout << endl;
            cout << "GitHub Repository:" << endl;
            cout << "https://github.com/lackyhy/7SCW" << endl;
            cout << endl;
            cout << "Description:" << endl;
            cout << "A comprehensive system management tool for Windows" << endl;
            cout << "providing file management, system information," << endl;
            cout << "startup management, security tools, and more." << endl;
            cout << endl;
            cout << "Features:" << endl;
            cout << "- File Manager with advanced navigation" << endl;
            cout << "- Startup locations management" << endl;
            cout << "- System information display" << endl;
            cout << "- Security tools and log viewing" << endl;
            cout << "- Custom terminal with bash-like commands" << endl;
            cout << "- Automatic updates from GitHub" << endl;
            cout << endl;
            cout << "Press any key to continue...";
            _getch();
            system("cls");
        }
        else if (cmd == "clear" || cmd == "cls") {
            system("cls");
            cout << "=== Custom Terminal (Bash-like) ===" << endl;
            cout << "Type 'exit/q/quit' to return to main menu" << endl;
            cout << "Type 'help' for available commands and 'update_programm' for update programm" << endl;
            cout << "Type TAB for command auto-completion" << endl;
            cout << "===================================" << endl << endl;
        }
        else if (cmd == "pwd") {
            cout << currentDir << endl;
        }
        else if (cmd == "activate_") {
            Logger::info("Running activator menu");
            __main_();
        }
        else if (cmd == "cookie_") {
            Logger::info("Opening cookie cleaner");
            main_menu_cookie();
        }
        else if (cmd == "logs_") {
            Logger::info("Opening logs file");
            Logger::openLogFile();
        }
        else if (cmd.substr(0, 2) == "cd") {
            string path = command.substr(2);
            if (!path.empty() && path[0] == ' ') {
                path = path.substr(1);
            }
            if (path.empty() || path == "~") {
                path = "C:\\";
            }
            if (SetCurrentDirectoryA(path.c_str())) {
                cout << "Changed directory to: " << path << endl;
            } else {
                cout << "Error: Cannot change to directory " << path << endl;
            }
        }
        else if (cmd == "ls" || cmd == "dir") {
            string dirCommand = "dir";
            system(dirCommand.c_str());
        }
        else if (cmd.substr(0, 5) == "mkdir") {
            string dirName = command.substr(5);
            if (!dirName.empty() && dirName[0] == ' ') {
                dirName = dirName.substr(1);
            }
            if (!dirName.empty()) {
                Logger::info("Attempting to create directory: " + dirName);
                string mkdirCommand = "mkdir \"" + dirName + "\"";
                int result = system(mkdirCommand.c_str());
                if (result == 0) {
                    Logger::success("Successfully created directory: " + dirName);
                } else {
                    Logger::warning("Failed to create directory: " + dirName);
                    cout << "Error: Cannot create directory " << dirName << endl;
                }
            } else {
                Logger::warning("mkdir command used without directory name");
                cout << "Error: Directory name required" << endl;
            }
        }
        else if (cmd.substr(0, 2) == "rm") {
            string fileName = command.substr(2);
            if (!fileName.empty() && fileName[0] == ' ') {
                fileName = fileName.substr(1);
            }
            if (!fileName.empty()) {
                Logger::info("Attempting to delete file: " + fileName);
                
                // Проверяем существование файла
                ifstream file(fileName);
                if (file.good()) {
                    file.close();
                    string delCommand = "del \"" + fileName + "\"";
                    int result = system(delCommand.c_str());
                    if (result == 0) {
                        Logger::success("Successfully deleted file: " + fileName);
                    } else {
                        Logger::warning("Failed to delete file: " + fileName);
                        cout << "Error: Cannot delete file " << fileName << endl;
                    }
                } else {
                    Logger::error("File not found: " + fileName);
                    cout << "Error: File not found: " << fileName << endl;
                }
            } else {
                Logger::warning("rm command used without filename");
                cout << "Error: File name required" << endl;
            }
        }
        else if (cmd.substr(0, 5) == "rmdir") {
            string dirName = command.substr(5);
            if (!dirName.empty() && dirName[0] == ' ') {
                dirName = dirName.substr(1);
            }
            if (!dirName.empty()) {
                Logger::info("Attempting to remove directory: " + dirName);
                
                // Проверяем существование директории
                DWORD attributes = GetFileAttributesA(dirName.c_str());
                if (attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY)) {
                    string rmdirCommand = "rmdir \"" + dirName + "\"";
                    int result = system(rmdirCommand.c_str());
                    if (result == 0) {
                        Logger::success("Successfully removed directory: " + dirName);
                    } else {
                        Logger::warning("Failed to remove directory: " + dirName);
                        cout << "Error: Cannot remove directory " << dirName << endl;
                    }
                } else {
                    Logger::error("Directory not found: " + dirName);
                    cout << "Error: Directory not found: " << dirName << endl;
                }
            } else {
                Logger::warning("rmdir command used without directory name");
                cout << "Error: Directory name required" << endl;
            }
        }
        else if (cmd.substr(0, 2) == "cp") {
            string args = command.substr(2);
            if (!args.empty() && args[0] == ' ') {
                args = args.substr(1);
            }
            if (!args.empty()) {
                Logger::info("Attempting to copy: " + args);
                string copyCommand = "copy " + args;
                int result = system(copyCommand.c_str());
                if (result == 0) {
                    Logger::success("Copy operation completed: " + args);
                } else {
                    Logger::warning("Copy operation failed: " + args);
                    cout << "Error: Copy operation failed" << endl;
                }
            } else {
                Logger::warning("cp command used without arguments");
                cout << "Error: Source and destination required" << endl;
            }
        }
        else if (cmd.substr(0, 2) == "mv") {
            string args = command.substr(2);
            if (!args.empty() && args[0] == ' ') {
                args = args.substr(1);
            }
            if (!args.empty()) {
                Logger::info("Attempting to move: " + args);
                string moveCommand = "move " + args;
                int result = system(moveCommand.c_str());
                if (result == 0) {
                    Logger::success("Move operation completed: " + args);
                } else {
                    Logger::warning("Move operation failed: " + args);
                    cout << "Error: Move operation failed" << endl;
                }
            } else {
                Logger::warning("mv command used without arguments");
                cout << "Error: Source and destination required" << endl;
            }
        }
        else if (cmd.substr(0, 3) == "cat") {
            string fileName = command.substr(3);
            if (!fileName.empty() && fileName[0] == ' ') {
                fileName = fileName.substr(1);
            }
            if (!fileName.empty()) {
                Logger::info("Attempting to read file: " + fileName);
                
                // Проверяем существование файла перед выполнением команды
                ifstream file(fileName);
                if (file.good()) {
                    file.close();
                    string typeCommand = "type \"" + fileName + "\"";
                    int result = system(typeCommand.c_str());
                    if (result != 0) {
                        Logger::warning("Failed to read file: " + fileName);
                        cout << "Error: Cannot read file " << fileName << endl;
                    } else {
                        Logger::success("Successfully read file: " + fileName);
                    }
                } else {
                    Logger::error("File not found: " + fileName);
                    cout << "Error: File not found: " << fileName << endl;
                }
            } else {
                Logger::warning("cat command used without filename");
                cout << "Error: File name required" << endl;
            }
        }
        else if (cmd.substr(0, 4) == "echo") {
            string text = command.substr(4);
            if (!text.empty() && text[0] == ' ') {
                text = text.substr(1);
            }
            cout << text << endl;
        }
        else if (cmd == "date" || cmd == "time") {
            cout << Logger::getCurrentTime() << endl;
        }
        else if (cmd == "whoami") {
            system("whoami");
        }
        else if (cmd == "hostname") {
            system("hostname");
        }
        else if (cmd == "systeminfo") {
            system("systeminfo");
        }
        else if (cmd == "processes") {
            system("tasklist");
        }
        else if (cmd == "services") {
            system("net start");
        }
        else if (cmd == "netstat") {
            system("netstat -an");
        }
        else if (cmd == "ipconfig") {
            system("ipconfig");
        }
        else if (cmd.substr(0, 11) == "create_hash") {
            string args = command.substr(11);
            if (!args.empty() && args[0] == ' ') {
                args = args.substr(1);
            }

            vector<string> tokens;
            stringstream ss(args);
            string token;
            while (ss >> token) {
                tokens.push_back(token);
            }

            createHash(tokens);
        }
        else if (cmd == "network") {
            showNetworkMenu();
        }
        else if (command.find("||") != string::npos) {
            // Chained command support: e.g., "search_file || SS::main.cpp"
            size_t delim = command.find("||");
            string left = command.substr(0, delim);
            string right = command.substr(delim + 2);
            // trim spaces
            auto trim = [](string s){
                size_t a = s.find_first_not_of(" \t");
                size_t b = s.find_last_not_of(" \t");
                if (a == string::npos) return string("");
                return s.substr(a, b - a + 1);
            };
            left = trim(left);
            right = trim(right);

            // Support: search_file || <pattern>
            if (left == "search_file") {
                if (!right.empty()) {
                    startSearchWithPattern(right, "");
                } else {
                    cout << "Pattern is empty" << endl;
                }
            } else {
                // Fallback: execute left, then right as raw commands
                system(left.c_str());
                if (!right.empty()) system(right.c_str());
            }
        }
        else if (cmd == "update_programm") {
            updateProgram();
        }
        else if (cmd == "search_file") {
            startSearch("");
        }
        else {
            system(command.c_str());
        }

        cout << endl;
    }

    system("cls");
    cout << "Returning to main menu..." << endl;
    Sleep(100);
}