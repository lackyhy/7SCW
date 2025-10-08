#ifndef FILE_HASH_VERIFIER_H
#define FILE_HASH_VERIFIER_H

#include <windows.h>
#include <string>
#include <vector>
#include <map>

struct FileHash {
    std::string filename;
    std::string md5;
    std::string sha1;
    std::string sha256;
    DWORD fileSize;
    FILETIME lastModified;
    bool isSystemFile;
};

struct VerificationResult {
    std::string filename;
    bool verified;
    std::string expectedHash;
    std::string actualHash;
    std::string status;
    std::string details;
};

class FileHashVerifier {
private:
    std::vector<FileHash> systemFiles;
    std::map<std::string, std::string> knownHashes;
    
    void loadKnownHashes();
    void scanSystemFiles();
    
public:
    std::string calculateMD5(const std::string& filename);
    std::string calculateSHA1(const std::string& filename);
    std::string calculateSHA256(const std::string& filename);
    FileHashVerifier();
    
    // Основные функции
    void verifySystemFiles();
    void verifySpecificFile(const std::string& filepath);
    void scanDirectory(const std::string& directory);
    
    // Утилиты
    void showVerificationResults();
    void exportResults(const std::string& filename);
    void addKnownHash(const std::string& filename, const std::string& hash);
    
    // Меню
    void showHashVerifierMenu();
};

// Функции для работы с хешами
std::string getFileHash(const std::string& filename, const std::string& algorithm);
bool verifyFileIntegrity(const std::string& filename, const std::string& expectedHash);
void scanCriticalSystemFiles();

#endif // FILE_HASH_VERIFIER_H
