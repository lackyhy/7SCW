#ifndef USERS_MANAGER_H
#define USERS_MANAGER_H

#include <string>
#include <vector>
#include <windows.h>

using namespace std;

struct UserAccountInfo {
    string username;
    string fullName;
    string comment;
    bool isDisabled;
    bool isLocked;
    bool isAdmin;
    string role;
    string lastLogon;
    DWORD passwordAgeDays;
    bool passwordRequired;
};

void showUsersManagerMenu();

#endif // USERS_MANAGER_H
