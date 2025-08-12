#include "terminal_commands.h"
#include <random>
#include <chrono>
#include <ctime>
#include <windows.h>
#include <iphlpapi.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <conio.h>

// Random generator initialization
void initializeRandomGenerator() {
    // Initialize random seed with high-resolution clock
    auto now = chrono::high_resolution_clock::now();
    auto seed = now.time_since_epoch().count();
    srand(static_cast<unsigned int>(seed));
}

// Simple XOR encryption/decryption
bool encryptFile(const string& filePath, const string& password) {
    ifstream inputFile(filePath, ios::binary);
    if (!inputFile.is_open()) {
        cout << "Error: Cannot open file for reading: " << filePath << endl;
        return false;
    }

    string outputPath = filePath + ".encrypted";
    ofstream outputFile(outputPath, ios::binary);
    if (!outputFile.is_open()) {
        cout << "Error: Cannot create encrypted file: " << outputPath << endl;
        return false;
    }

    char buffer[1024];
    size_t passwordIndex = 0;
    
    while (inputFile.read(buffer, sizeof(buffer))) {
        streamsize bytesRead = inputFile.gcount();
        for (streamsize i = 0; i < bytesRead; ++i) {
            buffer[i] ^= password[passwordIndex % password.length()];
            passwordIndex++;
        }
        outputFile.write(buffer, bytesRead);
    }

    inputFile.close();
    outputFile.close();
    
    cout << "File encrypted successfully: " << outputPath << endl;
    return true;
}

bool decryptFile(const string& filePath, const string& password) {
    ifstream inputFile(filePath, ios::binary);
    if (!inputFile.is_open()) {
        cout << "Error: Cannot open encrypted file: " << filePath << endl;
        return false;
    }

    string outputPath = filePath;
    if (outputPath.find(".encrypted") != string::npos) {
        outputPath = outputPath.substr(0, outputPath.find(".encrypted"));
    } else {
        outputPath += ".decrypted";
    }

    ofstream outputFile(outputPath, ios::binary);
    if (!outputFile.is_open()) {
        cout << "Error: Cannot create decrypted file: " << outputPath << endl;
        return false;
    }

    char buffer[1024];
    size_t passwordIndex = 0;
    
    while (inputFile.read(buffer, sizeof(buffer))) {
        streamsize bytesRead = inputFile.gcount();
        for (streamsize i = 0; i < bytesRead; ++i) {
            buffer[i] ^= password[passwordIndex % password.length()];
            passwordIndex++;
        }
        outputFile.write(buffer, bytesRead);
    }

    inputFile.close();
    outputFile.close();
    
    cout << "File decrypted successfully: " << outputPath << endl;
    return true;
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

void saveHashesToFile(const vector<string>& hashes, const string& filename) {
    ofstream file(filename);
    if (file.is_open()) {
        for (const string& hash : hashes) {
            file << hash << endl;
        }
        file.close();
        cout << "Hashes saved to: " << filename << endl;
    } else {
        cout << "Error: Cannot save hashes to file: " << filename << endl;
    }
}

void createHash(const vector<string>& args) {
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

// Command parsing and execution
bool executeCommand(const string& command, const vector<string>& args) {
    if (command == "encrypt") {
        if (args.size() >= 2) {
            return encryptFile(args[0], args[1]);
        } else {
            cout << "Usage: encrypt <filepath> <password>" << endl;
            return false;
        }
    } else if (command == "decrypt") {
        if (args.size() >= 1) {
            string password = args.size() >= 2 ? args[1] : "";
            return decryptFile(args[0], password);
        } else {
            cout << "Usage: decrypt <filepath> [password]" << endl;
            return false;
        }
    } else if (command == "create_hash") {
        createHash(args);
        return true;
    } else if (command == "network") {
        showNetworkMenu();
        return true;
    }
    
    return false;
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
        cout << "[6] Back to terminal" << endl;
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

void showSavedNetworks() {
    cout << "Saved Wi-Fi Networks:" << endl;
    cout << "====================" << endl;
    cout << endl;
    
    // Use netsh to get saved networks
    cout << "Executing: netsh wlan show profiles" << endl;
    cout << "-----------------------------------" << endl;
    system("netsh wlan show profiles");
    
    cout << endl;
    cout << "Note: Passwords are hidden for security reasons." << endl;
    cout << "To view a specific network password, use:" << endl;
    cout << "netsh wlan show profile name=\"NetworkName\" key=clear" << endl;
}

void showAvailableNetworks() {
    cout << "Available Wi-Fi Networks:" << endl;
    cout << "========================" << endl;
    cout << endl;
    
    cout << "Scanning for available networks..." << endl;
    cout << "----------------------------------" << endl;
    system("netsh wlan show networks");
    
    cout << endl;
    cout << "To connect to a network, use:" << endl;
    cout << "netsh wlan connect name=\"NetworkName\"" << endl;
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
