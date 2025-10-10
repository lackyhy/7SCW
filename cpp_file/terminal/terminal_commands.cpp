#include "../../h_file/terminal/terminal_commands.h"
#include "../../h_file/terminal/speed_test.h"
#include "../../h_file/main.h"
#include <random>
#include <chrono>
#include <ctime>
#include <windows.h>
#include <conio.h>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <iomanip>

// Global variables for update checking
bool updateCheckDone = false;
bool updateAvailable = false;

// Function to check for updates once at startup
void checkForUpdatesOnce() {
    if (!updateCheckDone) {
        string latestVersion = getLatestVersionFromGitHub();
        updateAvailable = (!latestVersion.empty() && compareVersions(VERSION, latestVersion));
        updateCheckDone = true;
    }
}

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

// Update functions
bool compareVersions(const string& currentVersion, const string& latestVersion) {
    // Simple version comparison (assumes format like "9.6.0")
    vector<int> currentParts, latestParts;
    
    // Parse current version
    stringstream currentSS(currentVersion);
    string part;
    while (getline(currentSS, part, '.')) {
        currentParts.push_back(stoi(part));
    }
    
    // Parse latest version
    stringstream latestSS(latestVersion);
    while (getline(latestSS, part, '.')) {
        latestParts.push_back(stoi(part));
    }
    
    // Compare versions
    size_t maxSize = max(currentParts.size(), latestParts.size());
    for (size_t i = 0; i < maxSize; ++i) {
        int currentPart = (i < currentParts.size()) ? currentParts[i] : 0;
        int latestPart = (i < latestParts.size()) ? latestParts[i] : 0;
        
        if (latestPart > currentPart) {
            return true; // New version available
        } else if (latestPart < currentPart) {
            return false; // Current version is newer
        }
    }
    
    return false; // Versions are equal
}

bool checkForUpdates(string& latestVersion, string& downloadUrl) {
    cout << "Checking for updates..." << endl;
    cout << "Connecting to GitHub repository..." << endl;
    
    // Get latest release info from GitHub API
    string command = "curl -s https://api.github.com/repos/lackyhy/7SCW/releases/latest > __temp_update_info.json";
    int result = system(command.c_str());
    
    if (result != 0) {
        cout << "Error: Failed to connect to GitHub. Check your internet connection." << endl;
        return false;
    }
    
    // Read the JSON response
    ifstream file("__temp_update_info.json");
    if (!file.is_open()) {
        cout << "Error: Cannot read update information." << endl;
        return false;
    }
    
    string jsonContent;
    string line;
    while (getline(file, line)) {
        jsonContent += line;
    }
    file.close();
    
    // Debug: Show first 500 characters of JSON
    cout << "Debug: JSON response preview: " << jsonContent.substr(0, 500) << "..." << endl;
    
    // Simple JSON parsing for version and download URL
    size_t versionPos = jsonContent.find("\"tag_name\"");
    
    if (versionPos == string::npos) {
        cout << "Error: Cannot parse version information." << endl;
        cout << "Debug: Full JSON content: " << jsonContent << endl;
        remove("__temp_update_info.json");
        return false;
    }
    
    // Extract version
    size_t versionStart = jsonContent.find("\"", versionPos + 10) + 1;
    size_t versionEnd = jsonContent.find("\"", versionStart);
    latestVersion = jsonContent.substr(versionStart, versionEnd - versionStart);
    
    // Extract download URL - try multiple methods
    downloadUrl = "";
    
    // Method 1: Look for browser_download_url in assets
    size_t assetsPos = jsonContent.find("\"assets\"");
    if (assetsPos != string::npos) {
        cout << "Debug: Found assets section" << endl;
        
        // Find the assets array content
        size_t assetsStart = jsonContent.find("[", assetsPos);
        size_t assetsEnd = jsonContent.find("]", assetsStart);
        
        if (assetsStart != string::npos && assetsEnd != string::npos) {
            string assetsContent = jsonContent.substr(assetsStart, assetsEnd - assetsStart);
            cout << "Debug: Assets content: " << assetsContent.substr(0, 200) << "..." << endl;
            
            size_t downloadUrlPos = assetsContent.find("\"browser_download_url\"");
            if (downloadUrlPos != string::npos) {
                cout << "Debug: Found browser_download_url in assets" << endl;
                size_t downloadStart = assetsContent.find("\"", downloadUrlPos + 21) + 1;
                size_t downloadEnd = assetsContent.find("\"", downloadStart);
                downloadUrl = assetsContent.substr(downloadStart, downloadEnd - downloadStart);
                cout << "Debug: Extracted URL: " << downloadUrl << endl;
            }
        }
    }
    
    // Method 2: Look for zipball_url or tarball_url
    if (downloadUrl.empty()) {
        size_t zipPos = jsonContent.find("\"zipball_url\"");
        if (zipPos != string::npos) {
            size_t zipStart = jsonContent.find("\"", zipPos + 13) + 1;
            size_t zipEnd = jsonContent.find("\"", zipStart);
            downloadUrl = jsonContent.substr(zipStart, zipEnd - zipStart);
            cout << "Debug: Using zipball_url: " << downloadUrl << endl;
        }
    }
    
    // Method 2.5: Alternative parsing - look for any URL in assets
    if (downloadUrl.empty()) {
        cout << "Debug: Trying alternative URL parsing..." << endl;
        
        // Look for "url": "https://" pattern in assets
        size_t urlPatternPos = jsonContent.find("\"url\": \"https://");
        if (urlPatternPos != string::npos) {
            size_t urlStart = jsonContent.find("\"", urlPatternPos + 5) + 1;
            size_t urlEnd = jsonContent.find("\"", urlStart);
            string foundUrl = jsonContent.substr(urlStart, urlEnd - urlStart);
            
            // Check if this looks like a download URL
            if (foundUrl.find("github.com") != string::npos && 
                (foundUrl.find(".exe") != string::npos || foundUrl.find(".zip") != string::npos)) {
                downloadUrl = foundUrl;
                cout << "Debug: Found download URL via alternative method: " << downloadUrl << endl;
            }
        }
    }
    
    // Method 3: Create a direct download URL based on version
    if (downloadUrl.empty()) {
        // Try multiple possible URLs for the release
        vector<string> possibleUrls = {
            "https://github.com/lackyhy/7SCW/releases/download/" + latestVersion + "/7SCW.exe",
            "https://github.com/lackyhy/7SCW/releases/download/" + latestVersion + "/7SCW.zip",
            "https://github.com/lackyhy/7SCW/archive/refs/tags/" + latestVersion + ".zip",
            "https://github.com/lackyhy/7SCW/releases/latest/download/7SCW.exe"
        };
        
        downloadUrl = possibleUrls[0]; // Use the first one as default
        cout << "Debug: Using constructed URL with version " << latestVersion << ": " << downloadUrl << endl;
        cout << "Debug: Other possible URLs:" << endl;
        for (size_t i = 1; i < possibleUrls.size(); ++i) {
            cout << "  " << possibleUrls[i] << endl;
        }
    }
    
    // Method 4: Fallback to latest release download
    if (downloadUrl.empty()) {
        downloadUrl = "https://github.com/lackyhy/7SCW/releases/latest/download/7SCW.exe";
        cout << "Debug: Using fallback URL: " << downloadUrl << endl;
    }
    
    // Clean up temp file
    remove("__temp_update_info.json");
    
    cout << "Latest version found: " << latestVersion << endl;
    cout << "Download URL: " << downloadUrl << endl;
    return true;
}

bool downloadUpdate(const string& url) {
    cout << "Downloading update..." << endl;
    cout << "URL: " << url << endl;
    
    // If URL is empty or malformed, try alternative URLs
    vector<string> urlsToTry = {url};
    if (url.empty() || url == ":" || url.length() < 10) {
        cout << "Debug: Primary URL is malformed, trying alternatives..." << endl;
        
        // Get the latest version dynamically
        string latestVersion = getLatestVersionFromGitHub();
        if (latestVersion.empty()) {
            latestVersion = "latest"; // Fallback if we can't get version
        }
        
        urlsToTry = {
            "https://github.com/lackyhy/7SCW/releases/download/" + latestVersion + "/7SCW.exe",
            "https://github.com/lackyhy/7SCW/releases/download/" + latestVersion + "/7SCW.zip",
            "https://github.com/lackyhy/7SCW/archive/refs/tags/" + latestVersion + ".zip",
            "https://github.com/lackyhy/7SCW/releases/latest/download/7SCW.exe"
        };
        
        cout << "Debug: Using dynamic version: " << latestVersion << endl;
    }
    
    string filename = ""; // Declare filename outside the loop
    
    for (size_t i = 0; i < urlsToTry.size(); ++i) {
        const string& currentUrl = urlsToTry[i];
        cout << "Trying URL " << (i + 1) << "/" << urlsToTry.size() << ": " << currentUrl << endl;
        
        // Determine file extension based on URL
        filename = "7SCW_update";
        if (currentUrl.find(".zip") != string::npos) {
            filename += ".zip";
        } else if (currentUrl.find(".exe") != string::npos) {
            filename += ".exe";
        } else {
            filename += ".zip"; // Default to zip
        }
        
        string command = "curl -L -o \"" + filename + "\" \"" + currentUrl + "\"";
        cout << "Command: " << command << endl;
        
        int result = system(command.c_str());
        
        if (result == 0) {
            cout << "Successfully downloaded from URL " << (i + 1) << "!" << endl;
            break; // Success, exit the loop
        } else {
            cout << "Failed to download from URL " << (i + 1) << ". curl exit code: " << result << endl;
            if (i == urlsToTry.size() - 1) {
                cout << "Error: All download attempts failed." << endl;
                return false;
            }
        }
    }
    
    // Check if file was downloaded
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Error: Update file not found after download: " << filename << endl;
        return false;
    }
    file.close();
    
    // If it's a zip file, extract it
    if (filename.find(".zip") != string::npos) {
        cout << "Extracting ZIP file..." << endl;
        string extractCommand = "powershell -command \"Expand-Archive -Path '" + filename + "' -DestinationPath 'update_temp' -Force\"";
        int extractResult = system(extractCommand.c_str());
        
        if (extractResult != 0) {
            cout << "Error: Failed to extract ZIP file." << endl;
            return false;
        }
        
        // Look for the main executable in the extracted folder
        string findExeCommand = "dir /s /b update_temp\\*.exe > __temp_exe_list.txt";
        system(findExeCommand.c_str());
        
        ifstream exeList("__temp_exe_list.txt");
        if (exeList.is_open()) {
            string exePath;
            getline(exeList, exePath);
            exeList.close();
            
            if (!exePath.empty()) {
                // Copy the found exe to our update location
                string copyCommand = "copy \"" + exePath + "\" 7SCW_update.exe";
                system(copyCommand.c_str());
                
                // Clean up
                system("rmdir /s /q update_temp");
                remove(filename.c_str());
                remove("__temp_exe_list.txt");
                
                cout << "Update extracted and prepared successfully!" << endl;
                return true;
            }
        }
        
        cout << "Error: No executable found in ZIP file." << endl;
        system("rmdir /s /q update_temp");
        remove(filename.c_str());
        remove("__temp_exe_list.txt");
        return false;
    }
    
    cout << "Update downloaded successfully!" << endl;
    return true;
}

void installUpdate() {
    cout << "Installing update..." << endl;
    
    // Create batch script to replace the executable
    ofstream script("update_installer.bat");
    if (!script.is_open()) {
        cout << "Error: Cannot create update installer." << endl;
        return;
    }
    
    script << "@echo off" << endl;
    script << "echo Installing 7SCW update..." << endl;
    script << "timeout /t 2 /nobreak > nul" << endl;
    script << "if exist \"7SCW.exe\" del \"7SCW.exe\"" << endl;
    script << "if exist \"7SCW_update.exe\" ren \"7SCW_update.exe\" \"7SCW.exe\"" << endl;
    script << "echo Update installed successfully!" << endl;
    script << "echo Starting updated program..." << endl;
    script << "timeout /t 2 /nobreak > nul" << endl;
    script << "start \"\" \"7SCW.exe\"" << endl;
    script << "del \"update_installer.bat\"" << endl;
    script << "exit /b" << endl;
    
    script.close();
    
    cout << "Starting update installer..." << endl;
    cout << "The program will restart automatically after update." << endl;
    
    // Start the update installer with minimized window that will close automatically
    system("start /min update_installer.bat");
    
    // Exit current program
    exit(0);
}

void updateProgram() {
    system("cls");
    cout << "=========================================" << endl;
    cout << "            PROGRAM UPDATE" << endl;
    cout << "=========================================" << endl;
    cout << endl;
    
    // Current version (from main.h or define here)
    cout << "Current version: " << VERSION << endl;
    cout << endl;
    
    string latestVersion, downloadUrl;
    
    if (!checkForUpdates(latestVersion, downloadUrl)) {
        cout << "Failed to check for updates." << endl;
        cout << endl << "Press any key to continue...";
        _getch();
        return;
    }
    
    if (!compareVersions(VERSION, latestVersion)) {
        cout << "You are using the latest version!" << endl;
        cout << "Current: " << VERSION << endl;
        cout << "Latest:  " << latestVersion << endl;
        cout << endl << "Press any key to continue...";
        _getch();
        return;
    }
    
    cout << "New version available!" << endl;
    cout << "Current: " << VERSION << endl;
    cout << "Latest:  " << latestVersion << endl;
    cout << endl;
    
    cout << "Do you want to update now? (y/n): ";
    char choice = _getch();
    cout << endl;
    
    if (choice == 'y' || choice == 'Y') {
        if (downloadUpdate(downloadUrl)) {
            cout << endl;
            cout << "Update downloaded. Do you want to install it now? (y/n): ";
            char installChoice = _getch();
            cout << endl;
            
            if (installChoice == 'y' || installChoice == 'Y') {
                installUpdate();
            } else {
                cout << "Update downloaded but not installed." << endl;
                cout << "Run 'update_programm' again to install it." << endl;
            }
        }
    } else {
        cout << "Update cancelled." << endl;
    }
    
    cout << endl << "Press any key to continue...";
    _getch();
}

string getLatestVersionFromGitHub() {
    string latestVersion = "";
    
    // Check if curl is available
    if (system("curl --version > nul 2>&1") != 0) {
        return ""; // curl not available
    }
    
    // Get latest release info from GitHub API
    string command = "curl -s https://api.github.com/repos/lackyhy/7SCW/releases/latest > __temp_version_check.json";
    int result = system(command.c_str());
    
    if (result != 0) {
        return ""; // Failed to connect
    }
    
    // Read the JSON response
    ifstream file("__temp_version_check.json");
    if (!file.is_open()) {
        return "";
    }
    
    string jsonContent;
    string line;
    while (getline(file, line)) {
        jsonContent += line;
    }
    file.close();
    
    // Parse version
    size_t versionPos = jsonContent.find("\"tag_name\"");
    if (versionPos != string::npos) {
        size_t versionStart = jsonContent.find("\"", versionPos + 10) + 1;
        size_t versionEnd = jsonContent.find("\"", versionStart);
        latestVersion = jsonContent.substr(versionStart, versionEnd - versionStart);
    }
    
    // Clean up temp file
    remove("__temp_version_check.json");
    
    return latestVersion;
}

bool isUpdateAvailable() {
    // If we already checked, return cached result
    if (updateCheckDone) {
        return updateAvailable;
    }
    
    // Otherwise, check now and cache the result
    checkForUpdatesOnce();
    return updateAvailable;
}
