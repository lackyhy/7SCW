#include <windows.h>
#include <lm.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <sstream>
#include <conio.h>

#include "../../Logger.h"
#include "../../h_file/users/users_manager.h"

using namespace std;

#pragma comment(lib, "netapi32.lib")
#pragma comment(lib, "user32.lib")

// --- String Conversion Helpers ---

static wstring toWString(const string& str) {
    if (str.empty()) return L"";
    int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);
    vector<wchar_t> wbuf(len);
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, wbuf.data(), len);
    return wstring(wbuf.data());
}

static string toString(const wchar_t* wstr) {
    if (!wstr) return "";
    int len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
    vector<char> buf(len);
    WideCharToMultiByte(CP_ACP, 0, wstr, -1, buf.data(), len, NULL, NULL);
    return string(buf.data());
}

static void setColor(WORD color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

// --- NetUser API Data Gathering ---

static bool checkIfUserIsAdmin(const wstring& wUsername) {
    LPLOCALGROUP_USERS_INFO_0 pGroups = NULL;
    DWORD entriesRead = 0, totalEntries = 0;
    NET_API_STATUS status = NetUserGetLocalGroups(NULL, wUsername.c_str(), 0, LG_INCLUDE_INDIRECT, (LPBYTE*)&pGroups, MAX_PREFERRED_LENGTH, &entriesRead, &totalEntries);
    
    bool isAdmin = false;
    if (status == NERR_Success && pGroups != NULL) {
        for (DWORD i = 0; i < entriesRead; i++) {
            wstring groupName = pGroups[i].lgrui0_name;
            if (groupName == L"Administrators" || groupName == L"Администраторы") {
                isAdmin = true;
                break;
            }
        }
        NetApiBufferFree(pGroups);
    }
    return isAdmin;
}

static vector<UserAccountInfo> getAllUserAccounts() {
    vector<UserAccountInfo> users;
    
    LPUSER_INFO_1 pBuf = NULL;
    DWORD entriesRead = 0, totalEntries = 0, resumeHandle = 0;
    
    NET_API_STATUS nStatus = NetUserEnum(NULL, 1, FILTER_NORMAL_ACCOUNT, (LPBYTE*)&pBuf, MAX_PREFERRED_LENGTH, &entriesRead, &totalEntries, &resumeHandle);
    
    if (nStatus == NERR_Success || nStatus == ERROR_MORE_DATA) {
        LPUSER_INFO_1 pTmpBuf = pBuf;
        for (DWORD i = 0; i < entriesRead; i++) {
            if (pTmpBuf == NULL) break;
            
            UserAccountInfo info;
            info.username = toString(pTmpBuf->usri1_name);
            info.comment = toString(pTmpBuf->usri1_comment);
            info.isDisabled = (pTmpBuf->usri1_flags & UF_ACCOUNTDISABLE) != 0;
            info.isLocked = (pTmpBuf->usri1_flags & UF_LOCKOUT) != 0;
            info.passwordRequired = (pTmpBuf->usri1_flags & UF_PASSWD_NOTREQD) == 0;
            info.passwordAgeDays = pTmpBuf->usri1_password_age / (24 * 3600);
            
            wstring wName = toWString(info.username);
            
            // Detailed Info Level 2
            LPUSER_INFO_2 pBuf2 = NULL;
            if (NetUserGetInfo(NULL, wName.c_str(), 2, (LPBYTE*)&pBuf2) == NERR_Success && pBuf2 != NULL) {
                info.fullName = toString(pBuf2->usri2_full_name);
                
                if (pBuf2->usri2_last_logon > 0) {
                    time_t t = (time_t)pBuf2->usri2_last_logon;
                    char timeBuf[64] = {0};
                    strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", localtime(&t));
                    info.lastLogon = timeBuf;
                } else {
                    info.lastLogon = "Never";
                }
                NetApiBufferFree(pBuf2);
            } else {
                info.fullName = "";
                info.lastLogon = "Unknown";
            }
            
            info.isAdmin = checkIfUserIsAdmin(wName);
            if (info.isAdmin) {
                info.role = "Administrator";
            } else if (pTmpBuf->usri1_priv == USER_PRIV_GUEST || info.username == "Guest" || info.username == "Гость") {
                info.role = "Guest";
            } else {
                info.role = "Standard User";
            }
            
            users.push_back(info);
            pTmpBuf++;
        }
    }
    
    if (pBuf != NULL) {
        NetApiBufferFree(pBuf);
    }
    return users;
}

// --- Action Functions ---

static bool createNewUserAccount(const string& username, const string& password, const string& fullName, bool makeAdmin) {
    wstring wUsername = toWString(username);
    wstring wPassword = toWString(password);
    wstring wFullName = toWString(fullName);
    
    USER_INFO_1 ui;
    ZeroMemory(&ui, sizeof(ui));
    ui.usri1_name = (LPWSTR)wUsername.c_str();
    ui.usri1_password = (LPWSTR)wPassword.c_str();
    ui.usri1_priv = USER_PRIV_USER;
    ui.usri1_flags = UF_SCRIPT | UF_DONT_EXPIRE_PASSWD;
    
    DWORD dwError = 0;
    NET_API_STATUS status = NetUserAdd(NULL, 1, (LPBYTE)&ui, &dwError);
    
    if (status != NERR_Success) {
        // Fallback to CLI command
        string cmd = "net user \"" + username + "\" \"" + password + "\" /add";
        system(cmd.c_str());
    }
    
    if (!fullName.empty()) {
        USER_INFO_1011 ui1011;
        ui1011.usri1011_full_name = (LPWSTR)wFullName.c_str();
        NetUserSetInfo(NULL, wUsername.c_str(), 1011, (LPBYTE)&ui1011, NULL);
    }
    
    if (makeAdmin) {
        LOCALGROUP_MEMBERS_INFO_3 lgmi;
        lgmi.lgrmi3_domainandname = (LPWSTR)wUsername.c_str();
        NetLocalGroupAddMembers(NULL, L"Administrators", 3, (LPBYTE)&lgmi, 1);
        
        string adminCmd = "net localgroup administrators \"" + username + "\" /add";
        system(adminCmd.c_str());
    }
    return true;
}

static bool changeUserPassword(const string& username, const string& newPassword) {
    wstring wUsername = toWString(username);
    wstring wPassword = toWString(newPassword);
    
    USER_INFO_1003 ui1003;
    ui1003.usri1003_password = (LPWSTR)wPassword.c_str();
    NET_API_STATUS status = NetUserSetInfo(NULL, wUsername.c_str(), 1003, (LPBYTE)&ui1003, NULL);
    
    if (status != NERR_Success) {
        string cmd = "net user \"" + username + "\" \"" + newPassword + "\"";
        int res = system(cmd.c_str());
        return (res == 0);
    }
    return true;
}

static bool setUserAccountDisabled(const string& username, bool disable) {
    wstring wUsername = toWString(username);
    
    LPUSER_INFO_2 pBuf = NULL;
    if (NetUserGetInfo(NULL, wUsername.c_str(), 2, (LPBYTE*)&pBuf) == NERR_Success && pBuf != NULL) {
        if (disable) {
            pBuf->usri2_flags |= UF_ACCOUNTDISABLE;
        } else {
            pBuf->usri2_flags &= ~UF_ACCOUNTDISABLE;
        }
        
        USER_INFO_1008 ui1008;
        ui1008.usri1008_flags = pBuf->usri2_flags;
        NetUserSetInfo(NULL, wUsername.c_str(), 1008, (LPBYTE)&ui1008, NULL);
        NetApiBufferFree(pBuf);
    }
    
    string cmd = "net user \"" + username + "\" /active:" + (disable ? "no" : "yes");
    system(cmd.c_str());
    return true;
}

static bool deleteUserAccount(const string& username) {
    wstring wUsername = toWString(username);
    NET_API_STATUS status = NetUserDel(NULL, wUsername.c_str());
    if (status != NERR_Success) {
        string cmd = "net user \"" + username + "\" /delete";
        int res = system(cmd.c_str());
        return (res == 0);
    }
    return true;
}

static bool setUserAdminStatus(const string& username, bool makeAdmin) {
    wstring wUsername = toWString(username);
    LOCALGROUP_MEMBERS_INFO_3 lgmi;
    lgmi.lgrmi3_domainandname = (LPWSTR)wUsername.c_str();
    
    if (makeAdmin) {
        NetLocalGroupAddMembers(NULL, L"Administrators", 3, (LPBYTE)&lgmi, 1);
        string cmd = "net localgroup administrators \"" + username + "\" /add";
        system(cmd.c_str());
    } else {
        NetLocalGroupDelMembers(NULL, L"Administrators", 3, (LPBYTE)&lgmi, 1);
        string cmd = "net localgroup administrators \"" + username + "\" /delete";
        system(cmd.c_str());
    }
    return true;
}

// --- Menu UI ---

static void drawUsersTable(const vector<UserAccountInfo>& users) {
    setColor(FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    cout << "================================================================================\n";
    cout << "                         WINDOWS USER ACCOUNTS MANAGER                          \n";
    cout << "================================================================================\n";
    setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

    cout << left << setw(4) << " # " 
         << setw(20) << "Username" 
         << setw(24) << "Full Name" 
         << setw(16) << "Role" 
         << setw(14) << "Status" << "\n";
    
    setColor(FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    cout << string(80, '-') << "\n";
    setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

    for (size_t i = 0; i < users.size(); ++i) {
        const auto& user = users[i];
        
        setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY); // Yellow index
        cout << " " << left << setw(3) << (to_string(i + 1) + ".");
        
        setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY); // White name
        cout << setw(20) << user.username;
        
        setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        string displayFullName = user.fullName.empty() ? "-" : user.fullName;
        if (displayFullName.length() > 22) displayFullName = displayFullName.substr(0, 19) + "...";
        cout << setw(24) << displayFullName;

        if (user.isAdmin) {
            setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY); // Yellow role for Admin
        } else {
            setColor(FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY); // Cyan role for User
        }
        cout << setw(16) << user.role;

        if (user.isDisabled) {
            setColor(FOREGROUND_RED | FOREGROUND_INTENSITY); // Red for Disabled
            cout << setw(14) << "[Disabled]";
        } else if (user.isLocked) {
            setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
            cout << setw(14) << "[Locked]";
        } else {
            setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY); // Green for Active
            cout << setw(14) << "[Active]";
        }
        cout << "\n";
        setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    }
    setColor(FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    cout << string(80, '=') << "\n";
    setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}

void showUsersManagerMenu() {
    bool running = true;
    while (running) {
        system("cls");
        Logger::info("Opening Users Manager Menu");
        
        vector<UserAccountInfo> users = getAllUserAccounts();
        drawUsersTable(users);
        
        setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        cout << "\nActions:\n";
        setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
        cout << "  1. View Account Details\n";
        cout << "  2. Create New User Account\n";
        cout << "  3. Change User Password\n";
        cout << "  4. Enable / Disable Account\n";
        cout << "  5. Change Privilege Role (Admin / Standard User)\n";
        cout << "  6. Delete User Account\n";
        cout << "  7. Refresh List\n";
        cout << "  8 (q). Back to Main Menu\n\n";
        
        setColor(FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
        cout << "Enter option (1-8): ";
        setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

        int option = 0;
        if (!(cin >> option)) {
            cin.clear();
            cin.ignore(10000, '\n');
            continue;
        }

        if (option == 8 || option == 'q') {
            running = false;
            break;
        } else if (option == 7) {
            continue;
        } else if (option == 1) { // View Details
            cout << "\nEnter user number (1-" << users.size() << "): ";
            int idx = 0;
            if (cin >> idx && idx >= 1 && (size_t)idx <= users.size()) {
                const auto& u = users[idx - 1];
                system("cls");
                setColor(FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
                cout << "==================================================\n";
                cout << "         USER ACCOUNT DETAILS: " << u.username << "\n";
                cout << "==================================================\n";
                setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
                cout << "  Username:           " << u.username << "\n";
                cout << "  Full Name:          " << (u.fullName.empty() ? "-" : u.fullName) << "\n";
                cout << "  Description:        " << (u.comment.empty() ? "-" : u.comment) << "\n";
                cout << "  Role Privilege:     " << u.role << "\n";
                cout << "  Account Status:     " << (u.isDisabled ? "Disabled" : (u.isLocked ? "Locked" : "Active")) << "\n";
                cout << "  Password Required:  " << (u.passwordRequired ? "Yes" : "No") << "\n";
                cout << "  Password Age:       " << u.passwordAgeDays << " days\n";
                cout << "  Last Logon:         " << u.lastLogon << "\n";
                setColor(FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
                cout << "==================================================\n";
                setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
                cout << "\nPress any key to continue...";
                _getch();
            }
        } else if (option == 2) { // Create User
            string newUsername, newPassword, newFullName;
            int adminChoice = 0;
            cout << "\n--- CREATE NEW USER --- \n";
            cout << "Enter new Username: ";
            cin >> newUsername;
            cout << "Enter Password: ";
            cin >> newPassword;
            cin.ignore(10000, '\n');
            cout << "Enter Full Name (optional, press Enter to skip): ";
            getline(cin, newFullName);
            cout << "Grant Administrator privileges? (1 = Yes, 0 = No): ";
            cin >> adminChoice;

            if (!newUsername.empty() && !newPassword.empty()) {
                Logger::info("Creating new user account: " + newUsername);
                createNewUserAccount(newUsername, newPassword, newFullName, adminChoice == 1);
                cout << "\n[+] User account created successfully!\n";
                cout << "Press any key to continue...";
                _getch();
            }
        } else if (option == 3) { // Change Password
            cout << "\nEnter user number (1-" << users.size() << "): ";
            int idx = 0;
            if (cin >> idx && idx >= 1 && (size_t)idx <= users.size()) {
                const auto& u = users[idx - 1];
                string newPass;
                cout << "Enter new password for " << u.username << ": ";
                cin >> newPass;
                if (!newPass.empty()) {
                    Logger::info("Changing password for user: " + u.username);
                    changeUserPassword(u.username, newPass);
                    cout << "\n[+] Password updated successfully!\n";
                    cout << "Press any key to continue...";
                    _getch();
                }
            }
        } else if (option == 4) { // Enable / Disable
            cout << "\nEnter user number (1-" << users.size() << "): ";
            int idx = 0;
            if (cin >> idx && idx >= 1 && (size_t)idx <= users.size()) {
                const auto& u = users[idx - 1];
                bool newDisabledState = !u.isDisabled;
                Logger::info((newDisabledState ? "Disabling" : "Enabling") + string(" user account: ") + u.username);
                setUserAccountDisabled(u.username, newDisabledState);
                cout << "\n[+] Account status changed to: " << (newDisabledState ? "Disabled" : "Active") << "\n";
                cout << "Press any key to continue...";
                _getch();
            }
        } else if (option == 5) { // Promote / Demote
            cout << "\nEnter user number (1-" << users.size() << "): ";
            int idx = 0;
            if (cin >> idx && idx >= 1 && (size_t)idx <= users.size()) {
                const auto& u = users[idx - 1];
                bool makeAdmin = !u.isAdmin;
                Logger::info((makeAdmin ? "Promoting to Admin" : "Demoting from Admin") + string(" user: ") + u.username);
                setUserAdminStatus(u.username, makeAdmin);
                cout << "\n[+] User role updated to: " << (makeAdmin ? "Administrator" : "Standard User") << "\n";
                cout << "Press any key to continue...";
                _getch();
            }
        } else if (option == 6) { // Delete User
            cout << "\nEnter user number (1-" << users.size() << "): ";
            int idx = 0;
            if (cin >> idx && idx >= 1 && (size_t)idx <= users.size()) {
                const auto& u = users[idx - 1];
                cout << "\n[!] WARNING: Are you sure you want to DELETE user '" << u.username << "'? (y/n): ";
                char confirm = 'n';
                cin >> confirm;
                if (confirm == 'y' || confirm == 'Y') {
                    Logger::info("Deleting user account: " + u.username);
                    deleteUserAccount(u.username);
                    cout << "\n[+] User account deleted!\n";
                    cout << "Press any key to continue...";
                    _getch();
                }
            }
        }
    }
}
