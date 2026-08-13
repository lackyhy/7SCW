#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <wininet.h>
#include <stdio.h>
#include <conio.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <sstream>
#include <cmath>

#include "../../Logger.h"
#include "../../h_file/system_info/system_info.h"

using namespace std;

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "wininet.lib")

// --- Helper UI Functions ---

static void setColor(WORD color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

static void printHeader(const string& title, int width = 80) {
    setColor(FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY); // Cyan
    string line(width, '=');
    cout << line << "\n";
    int padding = (width - (int)title.length()) / 2;
    if (padding < 0) padding = 0;
    cout << string(padding, ' ') << title << "\n";
    cout << line << "\n";
    setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); // Reset
}

static void printSectionHeader(const string& sectionName, int width = 80) {
    setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY); // Light Green
    cout << "\n[ " << sectionName << " ]\n";
    setColor(FOREGROUND_BLUE | FOREGROUND_INTENSITY); // Dark blue/cyan line
    cout << string(sectionName.length() + 5, '-') << "\n";
    setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}

static void printItem(const string& label, const string& value, int labelWidth = 32) {
    setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY); // White label
    cout << "  " << left << setw(labelWidth) << (label + ":");
    setColor(FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY); // Cyan value
    cout << value << "\n";
    setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}

static string getProgressBar(double percentage, int barWidth = 20) {
    if (percentage < 0.0) percentage = 0.0;
    if (percentage > 100.0) percentage = 100.0;
    
    int filled = (int)round((percentage / 100.0) * barWidth);
    string bar = "[";
    for (int i = 0; i < barWidth; ++i) {
        if (i < filled) bar += "=";
        else bar += "-";
    }
    bar += "] ";
    ostringstream ss;
    ss << fixed << setprecision(1) << percentage << "%";
    bar += ss.str();
    return bar;
}

// --- Data Collectors ---

static string getOSNameAndVersion() {
    string osName = "Windows";
    
    // Attempt to read Windows name from Registry
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        char productName[256] = {0};
        DWORD bufSize = sizeof(productName);
        if (RegQueryValueExA(hKey, "ProductName", NULL, NULL, (LPBYTE)productName, &bufSize) == ERROR_SUCCESS) {
            osName = productName;
        }
        
        char displayVersion[64] = {0};
        bufSize = sizeof(displayVersion);
        if (RegQueryValueExA(hKey, "DisplayVersion", NULL, NULL, (LPBYTE)displayVersion, &bufSize) == ERROR_SUCCESS) {
            osName += " (Version " + string(displayVersion) + ")";
        }
        
        char buildNumber[64] = {0};
        bufSize = sizeof(buildNumber);
        if (RegQueryValueExA(hKey, "CurrentBuildNumber", NULL, NULL, (LPBYTE)buildNumber, &bufSize) == ERROR_SUCCESS) {
            osName += " Build " + string(buildNumber);
        }
        RegCloseKey(hKey);
    } else {
        // Fallback to GetVersionEx
        OSVERSIONINFOEXA osVersionInfo;
        ZeroMemory(&osVersionInfo, sizeof(OSVERSIONINFOEXA));
        osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXA);
        if (GetVersionExA((OSVERSIONINFOA*)&osVersionInfo)) {
            ostringstream ss;
            ss << "Windows " << osVersionInfo.dwMajorVersion << "." << osVersionInfo.dwMinorVersion << " (Build " << osVersionInfo.dwBuildNumber << ")";
            osName = ss.str();
        }
    }
    return osName;
}

static string getCPUName() {
    string cpuName = "Unknown Processor";
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        char name[256] = {0};
        DWORD bufSize = sizeof(name);
        if (RegQueryValueExA(hKey, "ProcessorNameString", NULL, NULL, (LPBYTE)name, &bufSize) == ERROR_SUCCESS) {
            // Trim leading spaces
            string sName = name;
            size_t first = sName.find_first_not_of(' ');
            if (first != string::npos) {
                cpuName = sName.substr(first);
            } else {
                cpuName = sName;
            }
        }
        RegCloseKey(hKey);
    }
    return cpuName;
}

static string getUptimeString() {
    DWORD uptimeMs = GetTickCount();
    DWORD totalSec = uptimeMs / 1000;
    DWORD days = totalSec / (24 * 3600);
    DWORD hours = (totalSec % (24 * 3600)) / 3600;
    DWORD mins = (totalSec % 3600) / 60;
    DWORD secs = totalSec % 60;

    ostringstream ss;
    if (days > 0) ss << days << "d ";
    ss << hours << "h " << mins << "m " << secs << "s";
    return ss.str();
}

static string getExternalIPAddress() {
    string ip = "Unavailable";
    HINTERNET hInternet = InternetOpenA("7SCW-SysInfo", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (hInternet) {
        DWORD timeout = 2000; // 2 sec timeout
        InternetSetOptionA(hInternet, INTERNET_OPTION_CONNECT_TIMEOUT, &timeout, sizeof(timeout));
        InternetSetOptionA(hInternet, INTERNET_OPTION_RECEIVE_TIMEOUT, &timeout, sizeof(timeout));

        HINTERNET hConnect = InternetOpenUrlA(hInternet, "http://api.ipify.org", NULL, 0, INTERNET_FLAG_RELOAD, 0);
        if (hConnect) {
            char buffer[128];
            DWORD bytesRead = 0;
            if (InternetReadFile(hConnect, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead > 0) {
                buffer[bytesRead] = '\0';
                ip = buffer;
            }
            InternetCloseHandle(hConnect);
        }
        InternetCloseHandle(hInternet);
    }
    return ip;
}

static string getHWIDString() {
    DWORD dwSerial = 0;
    if (GetVolumeInformationA("C:\\", NULL, 0, &dwSerial, NULL, NULL, NULL, 0)) {
        ostringstream ss;
        ss << hex << uppercase << dwSerial;
        return ss.str();
    }
    return "Unknown";
}

// --- Section Display Logic ---

static void showOSAndIdentityInfo() {
    printSectionHeader("OPERATING SYSTEM & IDENTITY");

    printItem("OS Product Name", getOSNameAndVersion());

    char computerName[MAX_COMPUTERNAME_LENGTH + 1] = {0};
    DWORD compSize = sizeof(computerName);
    if (GetComputerNameA(computerName, &compSize)) {
        printItem("Computer Name", computerName);
    }

    char userName[256] = {0};
    DWORD userSize = sizeof(userName);
    if (GetUserNameA(userName, &userSize)) {
        printItem("Current User", userName);
    }

    printItem("Hardware ID (Volume Serial)", getHWIDString());
    printItem("System Uptime", getUptimeString());

    char sysDir[MAX_PATH] = {0};
    GetSystemDirectoryA(sysDir, MAX_PATH);
    printItem("System Directory", sysDir);

    char winDir[MAX_PATH] = {0};
    GetWindowsDirectoryA(winDir, MAX_PATH);
    printItem("Windows Directory", winDir);

    BOOL isWow64 = FALSE;
    typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
    LPFN_ISWOW64PROCESS fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandleA("kernel32"), "IsWow64Process");
    if (fnIsWow64Process != NULL) {
        fnIsWow64Process(GetCurrentProcess(), &isWow64);
        printItem("Architecture Mode", isWow64 ? "64-bit OS (WOW64 Emulation Process)" : "Native OS Mode");
    }
}

static void showCPUInfo() {
    printSectionHeader("PROCESSOR (CPU)");

    printItem("Model Name", getCPUName());

    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);

    ostringstream coresStream;
    coresStream << sysInfo.dwNumberOfProcessors << " Logical Threads";
    printItem("Logical Processors", coresStream.str());

    string archStr = "Unknown";
    switch (sysInfo.wProcessorArchitecture) {
        case PROCESSOR_ARCHITECTURE_AMD64: archStr = "x64 (64-bit)"; break;
        case PROCESSOR_ARCHITECTURE_INTEL: archStr = "x86 (32-bit)"; break;
        case PROCESSOR_ARCHITECTURE_ARM: archStr = "ARM"; break;
#ifdef PROCESSOR_ARCHITECTURE_ARM64
        case PROCESSOR_ARCHITECTURE_ARM64: archStr = "ARM64"; break;
#endif
    }
    printItem("CPU Architecture", archStr);

    ostringstream pageStream;
    pageStream << sysInfo.dwPageSize << " bytes";
    printItem("Page Size", pageStream.str());
}

static void showRAMInfo() {
    printSectionHeader("MEMORY (RAM)");

    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(MEMORYSTATUSEX);

    if (GlobalMemoryStatusEx(&memStatus)) {
        double totalGB = (double)memStatus.ullTotalPhys / (1024.0 * 1024.0 * 1024.0);
        double availGB = (double)memStatus.ullAvailPhys / (1024.0 * 1024.0 * 1024.0);
        double usedGB = totalGB - availGB;
        double loadPercent = (double)memStatus.dwMemoryLoad;

        ostringstream memTotalStr, memAvailStr, memUsedStr;
        memTotalStr << fixed << setprecision(2) << totalGB << " GB";
        memAvailStr << fixed << setprecision(2) << availGB << " GB";
        memUsedStr << fixed << setprecision(2) << usedGB << " GB (" << loadPercent << "% used)";

        printItem("Total Physical Memory", memTotalStr.str());
        printItem("Available Memory", memAvailStr.str());
        printItem("Memory Load", getProgressBar(loadPercent) + " (" + memUsedStr.str() + ")");

        double totalPageGB = (double)memStatus.ullTotalPageFile / (1024.0 * 1024.0 * 1024.0);
        double availPageGB = (double)memStatus.ullAvailPageFile / (1024.0 * 1024.0 * 1024.0);
        ostringstream pageStr;
        pageStr << fixed << setprecision(2) << (totalPageGB - availPageGB) << " / " << totalPageGB << " GB";
        printItem("Commit / Page File", pageStr.str());
    } else {
        printItem("RAM Status", "Failed to retrieve memory status");
    }
}

static void showGPUAndDisplayInfo() {
    printSectionHeader("GRAPHICS & DISPLAY (GPU)");

    DISPLAY_DEVICEA dd;
    ZeroMemory(&dd, sizeof(dd));
    dd.cb = sizeof(dd);

    bool foundGPU = false;
    for (DWORD i = 0; EnumDisplayDevicesA(NULL, i, &dd, 0); i++) {
        if (dd.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE) {
            printItem("Primary GPU", dd.DeviceString);
            foundGPU = true;
            break;
        }
    }
    if (!foundGPU) {
        if (EnumDisplayDevicesA(NULL, 0, &dd, 0)) {
            printItem("Display GPU", dd.DeviceString);
        } else {
            printItem("Display GPU", "Generic Display Device");
        }
    }

    DEVMODEA dm;
    ZeroMemory(&dm, sizeof(dm));
    dm.dmSize = sizeof(dm);
    if (EnumDisplaySettingsA(NULL, ENUM_CURRENT_SETTINGS, &dm)) {
        ostringstream resStr;
        resStr << dm.dmPelsWidth << " x " << dm.dmPelsHeight << " @ " << dm.dmDisplayFrequency << " Hz (" << dm.dmBitsPerPel << "-bit Color)";
        printItem("Screen Resolution", resStr.str());
    }
}

static void showDiskInfo() {
    printSectionHeader("STORAGE & DISK DRIVES");

    char driveBuffer[512] = {0};
    DWORD len = GetLogicalDriveStringsA(sizeof(driveBuffer) - 1, driveBuffer);
    if (len == 0) {
        printItem("Disk Storage", "Unable to enumerate drives");
        return;
    }

    char* drive = driveBuffer;
    while (*drive) {
        UINT driveType = GetDriveTypeA(drive);
        string typeStr = "Unknown";
        switch (driveType) {
            case DRIVE_FIXED: typeStr = "Local Fixed Disk"; break;
            case DRIVE_REMOVABLE: typeStr = "Removable Drive"; break;
            case DRIVE_REMOTE: typeStr = "Network Drive"; break;
            case DRIVE_CDROM: typeStr = "CD/DVD Drive"; break;
            case DRIVE_RAMDISK: typeStr = "RAM Disk"; break;
        }

        char volumeName[256] = {0};
        char fileSysName[256] = {0};
        DWORD serialNum = 0;
        GetVolumeInformationA(drive, volumeName, sizeof(volumeName), &serialNum, NULL, NULL, fileSysName, sizeof(fileSysName));

        ULARGE_INTEGER freeBytesAvailable, totalBytes, totalFreeBytes;
        if (GetDiskFreeSpaceExA(drive, &freeBytesAvailable, &totalBytes, &totalFreeBytes) && totalBytes.QuadPart > 0) {
            double totalGB = (double)totalBytes.QuadPart / (1024.0 * 1024.0 * 1024.0);
            double freeGB = (double)totalFreeBytes.QuadPart / (1024.0 * 1024.0 * 1024.0);
            double usedGB = totalGB - freeGB;
            double usedPercent = (usedGB / totalGB) * 100.0;

            ostringstream labelStream;
            labelStream << drive << " [" << (strlen(volumeName) > 0 ? volumeName : typeStr) << " (" << fileSysName << ")]";

            ostringstream detailStream;
            detailStream << fixed << setprecision(1) << usedGB << " GB / " << totalGB << " GB (" << freeGB << " GB free)";

            printItem(labelStream.str(), detailStream.str());
            cout << "                                  Usage: " << getProgressBar(usedPercent, 25) << "\n";
        }
        drive += strlen(drive) + 1;
    }
}

static void showNetworkInfo() {
    printSectionHeader("NETWORK & CONNECTIVITY");

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) == 0) {
        char hostName[256] = {0};
        if (gethostname(hostName, sizeof(hostName)) == 0) {
            printItem("Hostname", hostName);

            struct hostent* host = gethostbyname(hostName);
            if (host != NULL) {
                for (int i = 0; host->h_addr_list[i] != NULL; i++) {
                    struct in_addr addr;
                    memcpy(&addr, host->h_addr_list[i], sizeof(struct in_addr));
                    ostringstream ipLabel;
                    ipLabel << "Local IPv4 (" << (i + 1) << ")";
                    printItem(ipLabel.str(), inet_ntoa(addr));
                }
            }
        }
        WSACleanup();
    }

    printItem("External Public IP", getExternalIPAddress());

    // Enumerate Adapters
    IP_ADAPTER_INFO adapterInfo[16];
    ULONG dwBufLen = sizeof(adapterInfo);
    if (GetAdaptersInfo(adapterInfo, &dwBufLen) == NO_ERROR) {
        PIP_ADAPTER_INFO pAdapter = adapterInfo;
        int adapterIdx = 1;
        while (pAdapter && adapterIdx <= 4) { // Show up to top 4 active adapters
            if (strlen(pAdapter->IpAddressList.IpAddress.String) > 0 &&
                string(pAdapter->IpAddressList.IpAddress.String) != "0.0.0.0") {

                ostringstream macStream;
                for (UINT i = 0; i < pAdapter->AddressLength; i++) {
                    if (i > 0) macStream << "-";
                    macStream << hex << uppercase << setw(2) << setfill('0') << (int)pAdapter->Address[i];
                }

                ostringstream adLabel;
                adLabel << "Adapter " << adapterIdx << " (" << pAdapter->Description << ")";
                printItem("Adapter Name", pAdapter->Description);
                printItem("  MAC Address", macStream.str());
                printItem("  IP Address", pAdapter->IpAddressList.IpAddress.String);
                printItem("  Subnet Mask", pAdapter->IpAddressList.IpMask.String);
                printItem("  Gateway", pAdapter->GatewayList.IpAddress.String);
            }
            pAdapter = pAdapter->Next;
            adapterIdx++;
        }
    }
}

// --- Main Entry Point ---

void print_SystemInfo() {
    system("cls");
    Logger::info("Displaying enhanced system information menu");

    printHeader("SYSTEM INFORMATION & HARDWARE METRICS", 80);

    showOSAndIdentityInfo();
    showCPUInfo();
    showRAMInfo();
    showGPUAndDisplayInfo();
    showDiskInfo();
    showNetworkInfo();

    setColor(FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    cout << "\n================================================================================\n";
    setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY); // Yellow
    cout << "  [Press 'r' to Refresh | Press ANY OTHER KEY to return to main menu]\n";
    setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    cout << "================================================================================\n";

    int ch = _getch();
    if (ch == 'r' || ch == 'R') {
        print_SystemInfo();
    }
}