#ifndef FILE_HASH_VERIFIER_H
#define FILE_HASH_VERIFIER_H

#include <windows.h>
#include <string>
#include <vector>
#include <map>

using namespace std;

struct FileHash {
    string filename;
    string md5;
    string sha1;
    string sha256;
    DWORD fileSize;
    FILETIME lastModified;
    bool isSystemFile;
};

struct VerificationResult {
    string filename;
    bool verified;
    string expectedHash;
    string actualHash;
    string status;
    string details;
};

class FileHashVerifier {
private:
    vector<FileHash> systemFiles;
    map<string, string> knownHashes;
    
    void loadKnownHashes();
    void scanSystemFiles();
    
public:
    string calculateMD5(const string& filename);
    string calculateSHA1(const string& filename);
    string calculateSHA256(const string& filename);
    FileHashVerifier();
    
    // Основные функции
    void verifySystemFiles();
    void verifySpecificFile(const string& filepath);
    void scanDirectory(const string& directory);
    
    // Утилиты
    void showVerificationResults();
    void exportResults(const string& filename);
    void addKnownHash(const string& filename, const string& hash);
    
    // Меню
    void showHashVerifierMenu();
};

// Функции для работы с хешами
string getFileHash(const string& filename, const string& algorithm);
bool verifyFileIntegrity(const string& filename, const string& expectedHash);
void scanCriticalSystemFiles();

#endif // FILE_HASH_VERIFIER_H
