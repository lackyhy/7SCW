#ifndef PSTOOLS_MANAGER_H
#define PSTOOLS_MANAGER_H

#include <string>
#include <vector>

std::string getPSToolsDirectory();
std::string findPsExecPath();
bool isPSToolsInstalled();
bool downloadPSTools();
bool runAsSystem(const std::string& command, bool waitForExit = true);
bool isRunningAsSystem();
void showPSToolsMenu();

#endif // PSTOOLS_MANAGER_H
