#ifndef USERS_MANAGER_H
#define USERS_MANAGER_H

#include <string>
#include <vector>
#include <windows.h>

struct UserAccountInfo {
    std::string username;
    std::string fullName;
    std::string comment;
    bool isDisabled;
    bool isLocked;
    bool isAdmin;
    std::string role;
    std::string lastLogon;
    DWORD passwordAgeDays;
    bool passwordRequired;
};

void showUsersManagerMenu();

#endif // USERS_MANAGER_H
