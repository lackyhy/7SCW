#ifndef INC_7SCW_SHOW_ALL_STARTUP_H
#define INC_7SCW_SHOW_ALL_STARTUP_H

#include "string"

using namespace std;

class StartupManager {
private:
    void showProgress(const string& message, int current, int total);
    void collectStartupItems();
    void collectFolderStartupItems();
    void collectFilesFromFolder(const string& folderPath, const string& locationType);
    void collectRegistryStartupItems();
    void collectRegistryValues(const string& regPath, const string& locationType);
    void collectExtendedRegistryItems();
    void collectStartupApprovedItems();
    void collectScheduledTasks();
    void collectServices();
    void collectBrowserExtensions();
    void collectAppInitDLLs();
    void collectWinlogonItems();
    void collectPolicyItems();
    void displayItems();
    void showItemDetails();
    void deleteItem();
    void editItem();
    void openInRegedit();
    void openFileLocation();
    void handleItemActions();

public:
    void SHOW_ALL_STARTUP();
};

void startupMG_SHOW_ALL_STARTUP();

#endif //INC_7SCW_SHOW_ALL_STARTUP_H