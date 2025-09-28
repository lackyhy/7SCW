#ifndef TERMINAL_COMMANDS_H
#define TERMINAL_COMMANDS_H

#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <random>
#include <algorithm>
#include <cstring>

using namespace std;

// Random generator initialization
void initializeRandomGenerator();

// Hash creation functions
void createHash(const vector<string>& args);
void showHashHelp();
string generateRandomHash(int length);
void saveHashesToFile(const vector<string>& hashes, const string& filename);

// Network functions
void showNetworkMenu();
void testNetworkConnection();
void extendedPingTest();
void showSavedNetworks();
void showAvailableNetworks();
void pingSite(const string& url);

// Command parsing and execution
bool executeCommand(const string& command, const vector<string>& args);

#endif