#ifndef TERMINAL_COMMANDS_H
#define TERMINAL_COMMANDS_H

#include <iostream>
#include <string>
#include <vector>

// Hash creation functions
void createHash(const std::vector<std::string>& args);
void showHashHelp();
void saveHashesToFile(const std::vector<std::string>& hashes, const std::string& filename);
// Network functions
void showNetworkMenu();

void testNetworkConnection();
void extendedPingTest();
void showSavedNetworks();
void showAvailableNetworks();
void pingSite(const std::string& url);

std::string generateRandomHash(int length);

// Update functions
void updateProgram();
bool checkForUpdates(std::string& latestVersion, std::string& downloadUrl);
bool compareVersions(const std::string& currentVersion, const std::string& latestVersion);
bool downloadUpdate(const std::string& url);
void installUpdate();
std::string getLatestVersionFromGitHub();
bool isUpdateAvailable();
extern bool updateCheckDone;
extern bool updateAvailable;
void checkForUpdatesOnce();

#endif
