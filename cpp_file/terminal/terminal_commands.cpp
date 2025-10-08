#include "../../h_file/terminal/terminal_commands.h"
#include "../../h_file/terminal/speed_test.h"
#include <random>
#include <chrono>
#include <ctime>
#include <windows.h>
#include <conio.h>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <iomanip>

// Random generator initialization
void initializeRandomGenerator() {
    // Initialize random seed with high-resolution clock
    auto now = chrono::high_resolution_clock::now();
    auto seed = now.time_since_epoch().count();
    srand(static_cast<unsigned int>(seed));
}

// Hash creation functions
void showHashHelp() {
    cout << "Hash creation options:" << endl;
    cout << "  -l <length>    Set hash length (default: 32)" << endl;
    cout << "  -q <quantity>  Number of hashes to generate (default: 1)" << endl;
    cout << "  -s <seed>      Set random seed" << endl;
    cout << "  -f <filename>  Save hashes to file" << endl;
    cout << "  -h             Show this help" << endl;
    cout << endl;
    cout << "Examples:" << endl;
    cout << "  create_hash -l 64 -q 5" << endl;
    cout << "  create_hash -l 16 -q 10 -f hashes.txt" << endl;
}

string generateRandomHash(int length) {
    const string chars = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()_+-=[]{}|;:,.<>?";
    string hash;

    // Используем высококачественный генератор случайных чисел
    auto now = chrono::high_resolution_clock::now();
    auto seed = now.time_since_epoch().count();

    mt19937_64 gen(seed);
    uniform_int_distribution<int> dis(0, chars.length() - 1);

    for (int i = 0; i < length; ++i) {
        hash += chars[dis(gen)];
    }

    return hash;
}

void saveHashesToFile(const vector<string> &hashes, const string &filename) {
    ofstream file(filename);
    if (file.is_open()) {
        for (const string &hash : hashes) {
            file << hash << endl;
        }
        file.close();
        cout << "Hashes saved to: " << filename << endl;
    } else {
        cout << "Error: Cannot save hashes to file: " << filename << endl;
    }
}

void createHash(const vector<string> &args) {
    int length = 32;
    int quantity = 1;
    string filename = "";
    bool saveToFile = false;

    // Parse arguments
    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "-h" || args[i] == "--help") {
            showHashHelp();
            return;
        } else if (args[i] == "-l" && i + 1 < args.size()) {
            length = stoi(args[i + 1]);
            i++;
        } else if (args[i] == "-q" && i + 1 < args.size()) {
            quantity = stoi(args[i + 1]);
            i++;
        } else if (args[i] == "-f" && i + 1 < args.size()) {
            filename = args[i + 1];
            saveToFile = true;
            i++;
        }
    }

    // Generate hashes
    vector<string> hashes;
    for (int i = 0; i < quantity; ++i) {
        string hash = generateRandomHash(length);
        hashes.push_back(hash);
        cout << "Hash " << (i + 1) << ": " << hash << endl;
    }

    // Save to file if requested
    if (saveToFile && !filename.empty()) {
        saveHashesToFile(hashes, filename);
    }
}

// Network functions
void showNetworkMenu() {
    bool running = true;

    while (running) {
        system("cls");
        cout << "=========================================" << endl;
        cout << "              NETWORK MENU" << endl;
        cout << "=========================================" << endl;
        cout << endl;
        cout << "Choose an option:" << endl;
        cout << endl;
        cout << "[1] Test network connection" << endl;
        cout << "[2] Extended ping test (25 sites)" << endl;
        cout << "[3] Show saved networks" << endl;
        cout << "[4] Show available networks" << endl;
        cout << "[5] Ping specific site" << endl;
        cout << "[6] Speed test" << endl;
        cout << "[7] Back to terminal" << endl;
        cout << endl;
        cout << "Enter your choice (1-6): ";

        char choice = _getch();
        cout << endl << endl;

        switch (choice) {
            case '1':
                testNetworkConnection();
                break;
            case '2':
                extendedPingTest();
                break;
            case '3':
                showSavedNetworks();
                break;
            case '4':
                showAvailableNetworks();
                break;
            case '5':
            {
                cout << "Enter URL to ping (e.g., www.google.com): ";
                string url;
                getline(cin, url);
                if (!url.empty()) {
                    pingSite(url);
                }
            }
                break;
            case '6':
                speed_test();
                break;
            case '7':
                cout << "Returning to terminal..." << endl;
                running = false;
                break;
            default:
                cout << "Invalid choice!" << endl;
                break;
        }

        if (running) {
            cout << endl;
            cout << "Press any key to continue...";
            _getch();
        }
    }
}

void testNetworkConnection() {
    cout << "Testing your network connection..." << endl;
    cout << "==================================" << endl;
    cout << endl;

    // Test basic connectivity
    cout << "1. Testing basic connectivity..." << endl;
    if (system("ping -n 1 8.8.8.8 > nul 2>&1") == 0) {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        cout << "   [OK] Basic internet connectivity" << endl;
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    } else {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_INTENSITY);
        cout << "   [ERROR] No internet connectivity" << endl;
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    }

    // Test DNS resolution
    cout << "2. Testing DNS resolution..." << endl;
    if (system("ping -n 1 www.google.com > nul 2>&1") == 0) {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        cout << "   [OK] DNS resolution working" << endl;
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    } else {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_INTENSITY);
        cout << "   [ERROR] DNS resolution failed" << endl;
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    }

    // Test popular sites
    cout << "3. Testing popular websites..." << endl;
    string sites[] = {"www.google.com", "www.microsoft.com", "www.github.com", "www.telegram.org", "ru.wikipedia.org", "mail.ru"};
    for (const string& site : sites) {
        cout << "   Testing " << site << "... ";
        if (system(("ping -n 1 " + site + " > nul 2>&1").c_str()) == 0) {
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_INTENSITY);
            cout << "[OK]" << endl;
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        } else {
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_INTENSITY);
            cout << "[FAIL]" << endl;
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        }
    }
}

void extendedPingTest() {
    cout << "Extended Network Ping Test (25 sites)" << endl;
    cout << "=====================================" << endl;
    cout << endl;

    // Array of 25 popular websites
    string sites[] = {
            "www.google.com",
            "www.youtube.com",
            "www.facebook.com",
            "www.amazon.com",
            "www.netflix.com",
            "www.microsoft.com",
            "www.github.com",
            "www.stackoverflow.com",
            "www.reddit.com",
            "www.twitter.com",
            "www.instagram.com",
            "www.linkedin.com",
            "www.wikipedia.org",
            "www.apple.com",
            "www.cloudflare.com",
            "www.telegram.org",
            "www.vk.com",
            "www.yandex.ru",
            "www.mail.ru",
            "www.rambler.ru",
            "www.ozon.ru",
            "www.avito.ru",
            "www.2gis.ru",
            "www.kinopoisk.ru",
            "www.spotify.com"
    };

    int successCount = 0;
    int totalSites = 25;

    cout << "Testing connectivity to 25 popular websites..." << endl;
    cout << "==============================================" << endl;
    cout << endl;

    for (int i = 0; i < totalSites; i++) {
        cout << "[" << (i + 1) << "/" << totalSites << "] Testing " << sites[i] << "... ";

        if (system(("ping -n 1 " + sites[i] + " > nul 2>&1").c_str()) == 0) {
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_INTENSITY);
            cout << "[OK]" << endl;
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            successCount++;
        } else {
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_INTENSITY);
            cout << "[FAIL]" << endl;
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        }
    }

    cout << endl;
    cout << "==============================================" << endl;
    cout << "Extended Ping Test Results:" << endl;
    cout << "==============================================" << endl;

    double successRate = (double)successCount / totalSites * 100;

    cout << "Total sites tested: " << totalSites << endl;
    cout << "Successful connections: ";
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    cout << successCount;
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    cout << endl;

    cout << "Failed connections: ";
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_INTENSITY);
    cout << (totalSites - successCount);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    cout << endl;

    cout << "Success rate: ";
    if (successRate >= 80) {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    } else if (successRate >= 60) {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    } else {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_INTENSITY);
    }
    cout << fixed << setprecision(1) << successRate << "%";
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    cout << endl;

    cout << "==============================================" << endl;
}

// Helper function to get console width
int getConsoleWidth() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    return csbi.srWindow.Right - csbi.srWindow.Left + 1;
}

// Helper function to pad text to specific width
string padText(const string& text, int width, bool leftAlign = true) {
    if (text.length() >= width) return text.substr(0, width);
    if (leftAlign) {
        return text + string(width - text.length(), ' ');
    } else {
        return string(width - text.length(), ' ') + text;
    }
}

// Helper function to create a line separator
string createSeparator(int width, char character = '-') {
    return string(width, character);
}

void showSavedNetworks() {
    system("cls");
    cout << "=========================================" << endl;
    cout << "           SAVED WI-FI NETWORKS" << endl;
    cout << "=========================================" << endl;
    cout << endl;

    // Get console width for proper formatting
    int consoleWidth = getConsoleWidth();
    int tableWidth = min(consoleWidth - 4, 80); // Leave some margin

    // Create table header
    string headerSeparator = createSeparator(tableWidth, '=');
    string columnSeparator = createSeparator(tableWidth, '-');

    cout << headerSeparator << endl;

    // Calculate column widths
    int nameWidth = tableWidth * 3 / 5; // 60% for name
    int passwordWidth = tableWidth * 2 / 5; // 40% for password

    // Print header
    cout << "| " << padText("NETWORK NAME", nameWidth) << " | " << padText("PASSWORD", passwordWidth) << " |" << endl;
    cout << columnSeparator << endl;

    // Get saved networks using netsh
    FILE* pipe = _popen("netsh wlan show profiles", "r");
    if (!pipe) {
        cout << "Error: Cannot execute netsh command" << endl;
        return;
    }

    vector<string> networkNames;
    char buffer[128];
    string result = "";

    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        result += buffer;
    }
    _pclose(pipe);

    // Parse network names from the output
    stringstream ss(result);
    string line;
    while (getline(ss, line)) {
        if (line.find(":") != string::npos) {
            size_t colonPos = line.find(":");
            if (colonPos != string::npos) {
                string networkName = line.substr(colonPos + 1);
                // Trim whitespace
                networkName.erase(0, networkName.find_first_not_of(" \t\r\n"));
                networkName.erase(networkName.find_last_not_of(" \t\r\n") + 1);
                if (!networkName.empty()) {
                    networkNames.push_back(networkName);
                }
            }
        }
    }

    // Display networks in table format
    if (networkNames.empty()) {
        cout << "| " << padText("No saved networks found", tableWidth - 2) << " |" << endl;
    } else {
        for (const string& networkName : networkNames) {
            // Truncate network name if too long
            string displayName = networkName;
            if (displayName.length() > nameWidth - 2) {
                displayName = displayName.substr(0, nameWidth - 5) + "...";
            }

            cout << "| " << padText(displayName, nameWidth) << " | " << padText("********", passwordWidth) << " |" << endl;
        }
    }

    cout << headerSeparator << endl;
    cout << endl;

    // Show options
    cout << "Options:" << endl;
    cout << "1. View password for specific network" << endl;
    cout << "2. Delete network profile" << endl;
    cout << "3. Export networks list" << endl;
    cout << "4. Back to network menu" << endl;
    cout << endl;
    cout << "Enter choice (1-4): ";

    char choice = _getch();
    cout << endl;

    switch (choice) {
        case '1': {
            cout << "Enter network name to view password: ";
            string targetNetwork;
            getline(cin, targetNetwork);

            if (!targetNetwork.empty()) {
                cout << endl << "Getting password for: " << targetNetwork << endl;
                cout << "----------------------------------------" << endl;
                string command = "netsh wlan show profile name=\"" + targetNetwork + "\" key=clear";
                system(command.c_str());
            }
            break;
        }
        case '2': {
            cout << "Enter network name to delete: ";
            string targetNetwork;
            getline(cin, targetNetwork);

            if (!targetNetwork.empty()) {
                cout << endl << "Deleting network profile: " << targetNetwork << endl;
                cout << "----------------------------------------" << endl;
                string command = "netsh wlan delete profile name=\"" + targetNetwork + "\"";
                system(command.c_str());
                cout << "Profile deleted successfully!" << endl;
            }
            break;
        }
        case '3': {
            cout << "Exporting networks list to 'saved_networks.txt'..." << endl;
            system("netsh wlan show profiles > saved_networks.txt");
            cout << "Networks list exported successfully!" << endl;
            break;
        }
        case '4':
            return;
        default:
            cout << "Invalid choice!" << endl;
            break;
    }

    cout << endl << "Press any key to continue...";
    _getch();
}

void showAvailableNetworks() {
    system("cls");
    cout << "=========================================" << endl;
    cout << "         AVAILABLE WI-FI NETWORKS" << endl;
    cout << "=========================================" << endl;
    cout << endl;

    // Get console width for proper formatting
    int consoleWidth = getConsoleWidth();
    int tableWidth = min(consoleWidth - 4, 80); // Leave some margin

    // Create table header
    string headerSeparator = createSeparator(tableWidth, '=');
    string columnSeparator = createSeparator(tableWidth, '-');

    cout << headerSeparator << endl;

    // Calculate column widths
    int nameWidth = tableWidth * 2 / 3; // 66% for name
    int signalWidth = tableWidth * 1 / 3; // 33% for signal

    // Print header
    cout << "| " << padText("NETWORK NAME", nameWidth) << " | " << padText("SIGNAL", signalWidth) << " |" << endl;
    cout << columnSeparator << endl;

    // Get available networks using netsh
    FILE* pipe = _popen("netsh wlan show networks", "r");
    if (!pipe) {
        cout << "Error: Cannot execute netsh command" << endl;
        return;
    }

    vector<pair<string, string>> networks; // name, signal
    char buffer[128];
    string result = "";

    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        result += buffer;
    }
    _pclose(pipe);

    // Parse network information from the output
    stringstream ss(result);
    string line;
    string currentNetwork = "";
    string currentSignal = "";

    while (getline(ss, line)) {
        if (line.find("SSID") != string::npos && line.find(":") != string::npos) {
            size_t colonPos = line.find(":");
            if (colonPos != string::npos) {
                currentNetwork = line.substr(colonPos + 1);
                // Trim whitespace
                currentNetwork.erase(0, currentNetwork.find_first_not_of(" \t\r\n"));
                currentNetwork.erase(currentNetwork.find_last_not_of(" \t\r\n") + 1);
            }
        } else if (line.find("Signal") != string::npos && line.find(":") != string::npos) {
            size_t colonPos = line.find(":");
            if (colonPos != string::npos) {
                currentSignal = line.substr(colonPos + 1);
                // Trim whitespace
                currentSignal.erase(0, currentSignal.find_first_not_of(" \t\r\n"));
                currentSignal.erase(currentSignal.find_last_not_of(" \t\r\n") + 1);

                // If we have both network name and signal, add to list
                if (!currentNetwork.empty() && !currentSignal.empty()) {
                    networks.push_back(make_pair(currentNetwork, currentSignal));
                    currentNetwork = "";
                    currentSignal = "";
                }
            }
        }
    }

    // Display networks in table format
    if (networks.empty()) {
        cout << "| " << padText("No available networks found", tableWidth - 2) << " |" << endl;
    } else {
        for (const auto& network : networks) {
            // Truncate network name if too long
            string displayName = network.first;
            if (displayName.length() > nameWidth - 2) {
                displayName = displayName.substr(0, nameWidth - 5) + "...";
            }

            // Color code signal strength
            string signal = network.second;
            if (signal.find("Excellent") != string::npos || signal.find("Very Good") != string::npos) {
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_INTENSITY);
            } else if (signal.find("Good") != string::npos || signal.find("Fair") != string::npos) {
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
            } else {
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_INTENSITY);
            }

            cout << "| " << padText(displayName, nameWidth) << " | " << padText(signal, signalWidth) << " |" << endl;
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        }
    }

    cout << headerSeparator << endl;
    cout << endl;

    // Show options
    cout << "Options:" << endl;
    cout << "1. Connect to specific network" << endl;
    cout << "2. Refresh network list" << endl;
    cout << "3. Export networks list" << endl;
    cout << "4. Back to network menu" << endl;
    cout << endl;
    cout << "Enter choice (1-4): ";

    char choice = _getch();
    cout << endl;

    switch (choice) {
        case '1': {
            cout << "Enter network name to connect: ";
            string targetNetwork;
            getline(cin, targetNetwork);

            if (!targetNetwork.empty()) {
                cout << endl << "Connecting to: " << targetNetwork << endl;
                cout << "----------------------------------------" << endl;
                string command = "netsh wlan connect name=\"" + targetNetwork + "\"";
                system(command.c_str());
                cout << "Connection attempt completed!" << endl;
            }
            break;
        }
        case '2': {
            cout << "Refreshing network list..." << endl;
            showAvailableNetworks(); // Recursive call to refresh
            return;
        }
        case '3': {
            cout << "Exporting networks list to 'available_networks.txt'..." << endl;
            system("netsh wlan show networks > available_networks.txt");
            cout << "Networks list exported successfully!" << endl;
            break;
        }
        case '4':
            return;
        default:
            cout << "Invalid choice!" << endl;
            break;
    }

    cout << endl << "Press any key to continue...";
    _getch();
}

void pingSite(const string& url) {
    cout << "Pinging " << url << "..." << endl;
    cout << "=======================" << endl;
    cout << endl;

    // First do a quick test to determine color
    string testCommand = "ping -n 1 " + url + " > nul 2>&1";
    bool pingSuccess = (system(testCommand.c_str()) == 0);

    if (pingSuccess) {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        cout << "[OK] Site is reachable - showing detailed ping results:" << endl;
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    } else {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_INTENSITY);
        cout << "[ERROR] Site is not reachable - showing detailed ping results:" << endl;
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    }
    cout << endl;

    string command = "ping " + url;
    system(command.c_str());
}
