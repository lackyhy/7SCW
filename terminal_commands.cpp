#include "terminal_commands.h"

// Global random generator for better randomness
static mt19937 globalGenerator;

// Initialize random generator once
void initializeRandomGenerator() {
    static bool initialized = false;
    if (!initialized) {
        random_device rd;
        globalGenerator.seed(rd());
        initialized = true;
    }
}

// Simple XOR encryption/decryption
bool encryptFile(const string& filePath, const string& password) {
    ifstream inputFile(filePath, ios::binary);
    if (!inputFile.is_open()) {
        cout << "Error: Cannot open file " << filePath << endl;
        return false;
    }
    
    string outputPath = filePath + ".encrypted";
    ofstream outputFile(outputPath, ios::binary);
    if (!outputFile.is_open()) {
        cout << "Error: Cannot create encrypted file" << endl;
        inputFile.close();
        return false;
    }
    
    char buffer[1024];
    int passwordIndex = 0;
    
    while (inputFile.read(buffer, sizeof(buffer))) {
        int bytesRead = inputFile.gcount();
        for (int i = 0; i < bytesRead; i++) {
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
    if (password.empty()) {
        cout << "Enter password for decryption: ";
        string inputPassword;
        getline(cin, inputPassword);
        if (inputPassword.empty()) {
            cout << "Error: Password cannot be empty" << endl;
            return false;
        }
        return decryptFile(filePath, inputPassword);
    }
    
    ifstream inputFile(filePath, ios::binary);
    if (!inputFile.is_open()) {
        cout << "Error: Cannot open file " << filePath << endl;
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
        cout << "Error: Cannot create decrypted file" << endl;
        inputFile.close();
        return false;
    }
    
    char buffer[1024];
    int passwordIndex = 0;
    
    while (inputFile.read(buffer, sizeof(buffer))) {
        int bytesRead = inputFile.gcount();
        for (int i = 0; i < bytesRead; i++) {
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
string generateRandomHash(int length) {
    initializeRandomGenerator();
    
    const string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()_+-=[]{}|;:,.<>?";
    uniform_int_distribution<int> distribution(0, charset.length() - 1);
    
    string hash;
    for (int i = 0; i < length; i++) {
        hash += charset[distribution(globalGenerator)];
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
        cout << "Hashes saved to file: " << filename << endl;
    } else {
        cout << "Error: Cannot save to file " << filename << endl;
    }
}

void showHashHelp() {
    cout << "\ncreate_hash command usage:" << endl;
    cout << "  create_hash -l <length>           - Set hash length" << endl;
    cout << "  create_hash -q <quantity>         - Set number of hashes to generate" << endl;
    cout << "  create_hash -s                    - Use standard settings (length 27, overrides -l)" << endl;
    cout << "  create_hash -f <filename>         - Save hashes to file" << endl;
    cout << "  create_hash -h                    - Show this help" << endl;
    cout << "\nExamples:" << endl;
    cout << "  create_hash -l 18 -q 5            - Generate 5 hashes of length 18" << endl;
    cout << "  create_hash -l 18 -q 5 -f File.txt - Generate 5 hashes and save to File.txt" << endl;
    cout << "  create_hash -s -q 10              - Generate 10 hashes with standard length 27" << endl;
    cout << "  create_hash -s -l 20              - Generate 1 hash with standard length 27 (ignores -l)" << endl;
    cout << "\nDefault settings:" << endl;
    cout << "  Length: 27 characters" << endl;
    cout << "  Quantity: 1 hash" << endl;
    cout << "  Characters: a-z, A-Z, 0-9, !@#$%^&*()_+-=[]{}|;:,.<>?" << endl;
    cout << "\nNote: -s flag has priority over -l flag" << endl;
}

void createHash(const vector<string>& args) {
    int length = 27;  // Default length
    int quantity = 1; // Default quantity
    string filename = "";
    bool useStandard = false;
    
    // Parse arguments
    for (size_t i = 0; i < args.size(); i++) {
        if (args[i] == "-s" || args[i] == "--settings") {
            useStandard = true;
            length = 27;
        }
        else if (args[i] == "-l" || args[i] == "--length") {
            if (useStandard) {
                cout << "Warning: -s flag is active, ignoring -l flag" << endl;
                continue;
            }
            if (i + 1 < args.size()) {
                try {
                    length = stoi(args[i + 1]);
                    if (length <= 0) {
                        cout << "Error: Length must be positive" << endl;
                        return;
                    }
                } catch (...) {
                    cout << "Error: Invalid length value" << endl;
                    return;
                }
                i++; // Skip next argument
            } else {
                cout << "Error: Length value required after -l" << endl;
                return;
            }
        }
        else if (args[i] == "-q" || args[i] == "--quantity") {
            if (i + 1 < args.size()) {
                try {
                    quantity = stoi(args[i + 1]);
                    if (quantity <= 0) {
                        cout << "Error: Quantity must be positive" << endl;
                        return;
                    }
                } catch (...) {
                    cout << "Error: Invalid quantity value" << endl;
                    return;
                }
                i++; // Skip next argument
            } else {
                cout << "Error: Quantity value required after -q" << endl;
                return;
            }
        }
        else if (args[i] == "-f" || args[i] == "--file") {
            if (i + 1 < args.size()) {
                filename = args[i + 1];
                i++; // Skip next argument
            } else {
                cout << "Error: Filename required after -f" << endl;
                return;
            }
        }
        else if (args[i] == "-h" || args[i] == "--help") {
            showHashHelp();
            return;
        }
    }
    
    if (useStandard) {
        cout << "Using standard settings: length = 27" << endl;
    }
    
    cout << "Generating " << quantity << " hash(es) of length " << length << "..." << endl;
    
    vector<string> hashes;
    for (int i = 0; i < quantity; i++) {
        string hash = generateRandomHash(length);
        hashes.push_back(hash);
        cout << "Hash " << (i + 1) << ": " << hash << endl;
    }
    
    if (!filename.empty()) {
        saveHashesToFile(hashes, filename);
    }
}

bool executeCommand(const string& command, const vector<string>& args) {
    string cmd = command;
    transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);
    
    if (cmd == "encrypt") {
        if (args.size() >= 3 && args[0] == "-pass") {
            string filePath = args[2];
            string password = args[1];
            return encryptFile(filePath, password);
        } else {
            cout << "Usage: encrypt -pass <password> <filepath>" << endl;
            return false;
        }
    }
    else if (cmd == "deencrypt" || cmd == "decrypt") {
        if (args.size() >= 1) {
            string filePath = args[0];
            string password = "";
            if (args.size() >= 3 && args[1] == "-pass") {
                password = args[2];
            }
            return decryptFile(filePath, password);
        } else {
            cout << "Usage: deencrypt [filepath] or deencrypt -pass <password> <filepath>" << endl;
            return false;
        }
    }
    else if (cmd == "create_hash") {
        createHash(args);
        return true;
    }
    
    return false; // Command not handled by this module
}
