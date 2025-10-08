#ifndef TERMINAL_COMMANDS_H
#define TERMINAL_COMMANDS_H

#include <iostream>
#include <string>
#include <vector>

using namespace std;

// Hash creation functions
void createHash(const vector<string>& args);
void showHashHelp();
void saveHashesToFile(const vector<string>& hashes, const string& filename);
// Network functions
void showNetworkMenu();

void testNetworkConnection();
void extendedPingTest();
void showSavedNetworks();
void showAvailableNetworks();
void pingSite(const string& url);

string generateRandomHash(int length);

#endif
