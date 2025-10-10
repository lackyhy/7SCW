#include <iostream>
#include <windows.h>
#include <vector>
#include <string>
#include <algorithm>
#include <direct.h>

class ScriptManager {
private:
    std::string scriptsFolder;
    std::vector<std::string> scriptExtensions = {".cmd", ".bat", ".ps1"};

    bool isValidScriptExtension(const std::string& extension) {
        std::string ext = extension;
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        for (const auto& validExt : scriptExtensions) {
            if (ext == validExt) {
                return true;
            }
        }
        return false;
    }

public:
    ScriptManager(const std::string& folder = "__script_") : scriptsFolder(folder) {}

    bool checkScriptsFolder() {
        return (_access(scriptsFolder.c_str(), 0) == 0);
    }

    bool createScriptsFolder() {
        if (!checkScriptsFolder()) {
            return (_mkdir(scriptsFolder.c_str()) == 0);
        }
        return true;
    }

    std::vector<std::string> getScriptsList() {
        std::vector<std::string> scripts;

        if (!checkScriptsFolder()) {
            return scripts;
        }

        WIN32_FIND_DATAA findFileData;
        std::string searchPath = scriptsFolder + "\\*";
        HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findFileData);

        if (hFind == INVALID_HANDLE_VALUE) {
            return scripts;
        }

        do {
            if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                std::string fileName = findFileData.cFileName;
                std::string extension = fileName.substr(fileName.find_last_of("."));
                if (isValidScriptExtension(extension)) {
                    scripts.push_back(fileName);
                }
            }
        } while (FindNextFileA(hFind, &findFileData) != 0);

        FindClose(hFind);
        std::sort(scripts.begin(), scripts.end());
        return scripts;
    }

    std::string getScriptPath(const std::string& scriptName) {
        return scriptsFolder + "/" + scriptName;
    }

    bool scriptExists(const std::string& scriptName) {
        std::string fullPath = getScriptPath(scriptName);
        WIN32_FIND_DATAA findFileData;
        HANDLE hFind = FindFirstFileA(fullPath.c_str(), &findFileData);

        if (hFind == INVALID_HANDLE_VALUE) {
            return false;
        }

        FindClose(hFind);
        return true;
    }

    bool downloadScript(const std::string& scriptName) {
        std::string url = "https://raw.githubusercontent.com/lackyhy/__script_ms_/main/" + scriptName;
        std::string localPath = getScriptPath(scriptName);
        std::string command = "curl -s -o \"" + localPath + "\" \"" + url + "\"";

        int result = system(command.c_str());
        return (result == 0);
    }

    bool executeScript(const std::string& scriptName) {
        if (!scriptExists(scriptName)) {
            std::cout << "Script '" << scriptName << "' not found!\n";
            return false;
        }

        std::string fullPath = getScriptPath(scriptName);
        std::string extension = scriptName.substr(scriptName.find_last_of("."));
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

        std::string command;

        if (extension == ".ps1") {
            command = "powershell -ExecutionPolicy Bypass -File \"" + fullPath + "\"";
        } else {
            command = "\"" + fullPath + "\"";
        }

        std::cout << "Executing: " << command << "\n";
        std::cout << "----------------------------------------\n";

        int result = system(command.c_str());

        std::cout << "----------------------------------------\n";
        std::cout << "Script finished with code: " << result << "\n";

        return (result == 0);
    }
};

extern "C" {
__declspec(dllexport) std::vector<std::string> GetAvailableScripts() {
    ScriptManager manager;
    return manager.getScriptsList();
}

__declspec(dllexport) bool ExecuteScript(const std::string& scriptName) {
    ScriptManager manager;
    return manager.executeScript(scriptName);
}

__declspec(dllexport) bool CheckScriptsFolder() {
    ScriptManager manager;
    return manager.checkScriptsFolder();
}

__declspec(dllexport) bool DownloadScript(const std::string& scriptName) {
    ScriptManager manager;
    return manager.downloadScript(scriptName);
}
}