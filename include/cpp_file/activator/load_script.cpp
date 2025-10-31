#include <iostream>
#include <windows.h>
#include <vector>
#include <string>
#include <algorithm>
#include <direct.h>
#include "../../Logger.h"

using namespace std;

class ScriptManager {
private:
    string scriptsFolder;
    vector<string> scriptExtensions = {".cmd", ".bat", ".ps1"};

    bool isValidScriptExtension(const string& extension) {
        string ext = extension;
        transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        for (const auto& validExt : scriptExtensions) {
            if (ext == validExt) {
                return true;
            }
        }
        return false;
    }

public:
    ScriptManager(const string& folder = "__script_") : scriptsFolder(folder) {}

    bool checkScriptsFolder() {
        return (_access(scriptsFolder.c_str(), 0) == 0);
    }

    bool createScriptsFolder() {
        if (!checkScriptsFolder()) {
            return (_mkdir(scriptsFolder.c_str()) == 0);
        }
        return true;
    }

    vector<string> getScriptsList() {
        Logger::info("getScriptsList()");
        vector<string> scripts;

        if (!checkScriptsFolder()) {
            return scripts;
        }

        WIN32_FIND_DATAA findFileData;
        string searchPath = scriptsFolder + "\\*";
        HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findFileData);

        if (hFind == INVALID_HANDLE_VALUE) {
            return scripts;
        }

        do {
            if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                string fileName = findFileData.cFileName;
                string extension = fileName.substr(fileName.find_last_of("."));
                if (isValidScriptExtension(extension)) {
                    scripts.push_back(fileName);
                }
            }
        } while (FindNextFileA(hFind, &findFileData) != 0);

        FindClose(hFind);
        sort(scripts.begin(), scripts.end());
        return scripts;
    }

    string getScriptPath(const string& scriptName) {
        return scriptsFolder + "/" + scriptName;
    }

    bool scriptExists(const string& scriptName) {
        string fullPath = getScriptPath(scriptName);
        WIN32_FIND_DATAA findFileData;
        HANDLE hFind = FindFirstFileA(fullPath.c_str(), &findFileData);

        if (hFind == INVALID_HANDLE_VALUE) {
            return false;
        }

        FindClose(hFind);
        return true;
    }

    bool downloadScript(const string& scriptName) {
        Logger::info("start: downloadScript()");
        string url = "https://raw.githubusercontent.com/lackyhy/__script_ms_/main/" + scriptName;
        string localPath = getScriptPath(scriptName);
        Logger::functions_log(LOG_INFO, "downloadScript", "123");
        string command = "curl -s -o \"" + localPath + "\" \"" + url + "\"";

        int result = system(command.c_str());
        return (result == 0);
    }

    bool executeScript(const string& scriptName) {
        Logger::info("Executing script: " + scriptName);
        if (!scriptExists(scriptName)) {
            Logger::error("Script not found: " + scriptName);
            cout << "Script '" << scriptName << "' not found!\n";
            return false;
        }

        string fullPath = getScriptPath(scriptName);
        string extension = scriptName.substr(scriptName.find_last_of("."));
        transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

        string command;

        if (extension == ".ps1") {
            command = "powershell -ExecutionPolicy Bypass -File \"" + fullPath + "\"";
        } else {
            command = "\"" + fullPath + "\"";
        }

        cout << "Executing: " << command << "\n";
        cout << "----------------------------------------\n";

        int result = system(command.c_str());

        cout << "----------------------------------------\n";
        cout << "Script finished with code: " << result << "\n";

        return (result == 0);
    }
};

extern "C" {
__declspec(dllexport) vector<string> GetAvailableScripts() {
    ScriptManager manager;
    return manager.getScriptsList();
}

__declspec(dllexport) bool ExecuteScript(const string& scriptName) {
    ScriptManager manager;
    return manager.executeScript(scriptName);
}

__declspec(dllexport) bool CheckScriptsFolder() {
    ScriptManager manager;
    return manager.checkScriptsFolder();
}

__declspec(dllexport) bool DownloadScript(const string& scriptName) {
    ScriptManager manager;
    return manager.downloadScript(scriptName);
}
}