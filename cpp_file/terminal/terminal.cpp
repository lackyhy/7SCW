#include "iostream"
#include "string"
#include "vector"
#include "regex"

#include "windows.h"
#include "../../h_file/terminal/terminal_commands.h"
#include "../../h_file/terminal/terminal.h"
#include "../../h_file/activator/menu.h"

using namespace std;

void customTerminal() {
    system("cls");
    cout << "=== Custom Terminal (Bash-like) ===" << endl;
    cout << "Type 'exit/q/quit' to return to main menu" << endl;
    cout << "Type 'help' for available commands and 'update_programm' for update programm" << endl;
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

        // Get user input
        string command;
        getline(cin, command);

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
            break;
        }
        else if (cmd == "help") {
            cout << "\nAvailable commands:" << endl;
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
            cout << "  help             - Show this help" << endl;
            cout << "  exit, quit       - Exit terminal" << endl;
            cout << "  Any other command will be executed as Windows command" << endl << endl;
        }
        else if (cmd == "clear" || cmd == "cls") {
            system("cls");
            cout << "=== Custom Terminal (Bash-like) ===" << endl;
            cout << "Type 'exit/q/quit' to return to main menu" << endl;
            cout << "Type 'help' for available commands and 'update_programm' for update programm" << endl;
            cout << "===================================" << endl << endl;
        }
        else if (cmd == "pwd") {
            cout << currentDir << endl;
        }
        else if (cmd == "activate_") {
            __main_();
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
                string mkdirCommand = "mkdir \"" + dirName + "\"";
                system(mkdirCommand.c_str());
            } else {
                cout << "Error: Directory name required" << endl;
            }
        }
        else if (cmd.substr(0, 2) == "rm") {
            string fileName = command.substr(2);
            if (!fileName.empty() && fileName[0] == ' ') {
                fileName = fileName.substr(1);
            }
            if (!fileName.empty()) {
                string delCommand = "del \"" + fileName + "\"";
                system(delCommand.c_str());
            } else {
                cout << "Error: File name required" << endl;
            }
        }
        else if (cmd.substr(0, 5) == "rmdir") {
            string dirName = command.substr(5);
            if (!dirName.empty() && dirName[0] == ' ') {
                dirName = dirName.substr(1);
            }
            if (!dirName.empty()) {
                string rmdirCommand = "rmdir \"" + dirName + "\"";
                system(rmdirCommand.c_str());
            } else {
                cout << "Error: Directory name required" << endl;
            }
        }
        else if (cmd.substr(0, 2) == "cp") {
            string args = command.substr(2);
            if (!args.empty() && args[0] == ' ') {
                args = args.substr(1);
            }
            if (!args.empty()) {
                string copyCommand = "copy " + args;
                system(copyCommand.c_str());
            } else {
                cout << "Error: Source and destination required" << endl;
            }
        }
        else if (cmd.substr(0, 2) == "mv") {
            string args = command.substr(2);
            if (!args.empty() && args[0] == ' ') {
                args = args.substr(1);
            }
            if (!args.empty()) {
                string moveCommand = "move " + args;
                system(moveCommand.c_str());
            } else {
                cout << "Error: Source and destination required" << endl;
            }
        }
        else if (cmd.substr(0, 3) == "cat") {
            string fileName = command.substr(3);
            if (!fileName.empty() && fileName[0] == ' ') {
                fileName = fileName.substr(1);
            }
            if (!fileName.empty()) {
                string typeCommand = "type \"" + fileName + "\"";
                system(typeCommand.c_str());
            } else {
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
        else if (cmd == "date") {
            system("date /t");
            system("time /t");
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
            // Parse create_hash command
            string args = command.substr(11);
            if (!args.empty() && args[0] == ' ') {
                args = args.substr(1);
            }

            // Parse arguments into vector
            vector<string> tokens;
            stringstream ss(args);
            string token;
            while (ss >> token) {
                tokens.push_back(token);
            }

            // Use the function from terminal_commands module
            createHash(tokens);
        }
        else if (cmd == "network") {
            // Use the function from terminal_commands module
            showNetworkMenu();
        }
        else if (cmd == "update_programm") {
            // Use the function from terminal_commands module
            updateProgram();
        }
        else {
            // Execute as Windows command
            system(command.c_str());
        }

        cout << endl;
    }

    system("cls");
    cout << "Returning to main menu..." << endl;
    Sleep(100);
}
