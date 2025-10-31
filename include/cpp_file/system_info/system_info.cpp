#include <windows.h>
#include <stdio.h>
#include <iphlpapi.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <wininet.h>

#include "conio.h"
#include "../../Logger.h"

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "wininet.lib")

void print_Username()
{
    char username[256];
    DWORD size = sizeof(username);
    if (GetUserNameA(username, &size))
    {
        printf("    Username:                           %s\n", username);
    }
    else
    {
        printf("Username: Unable to retrieve\n");
        Logger::error("Failed to retrieve username");
    }
}

void print_ComputerName()
{
    char computername[256];
    DWORD size = sizeof(computername);
    if (GetComputerNameA(computername, &size))
    {
        printf("    Computer Name:                      %s\n", computername);
    }
    else
    {
        printf("    Computer Name: Unable to retrieve\n");
    }
}

void print_HWID()
{
    DWORD dwSerial;
    if (GetVolumeInformationA("C:\\", NULL, 0, &dwSerial, NULL, NULL, NULL, 0))
    {
        printf("    HWID:                               %lu\n", dwSerial);
    }
    else
    {
        printf("    HWID: Unable to retrieve\n");
    }
}

void print_ExternalIP()
{
    HINTERNET hInternet = InternetOpenA("IP Checker", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (hInternet)
    {
        HINTERNET hConnect = InternetOpenUrlA(hInternet, "http://api.ipify.org", NULL, 0, INTERNET_FLAG_RELOAD, 0);
        if (hConnect)
        {
            char buffer[256];
            DWORD bytesRead;
            if (InternetReadFile(hConnect, buffer, sizeof(buffer) - 1, &bytesRead))
            {
                buffer[bytesRead] = '\0';
                printf("    External IP: %s\n", buffer);
            }
            else
            {
                printf("    External IP: Unable to retrieve\n");
            }
            InternetCloseHandle(hConnect);
        }
        else
        {
            printf("    External IP: Unable to connect\n");
        }
        InternetCloseHandle(hInternet);
    }
    else
    {
        printf("    External IP: Internet unavailable\n");
    }
}

void print_IPAddresses()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("    IP Address: WSAStartup failed\n");
        return;
    }

    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == 0)
    {
        printf("    Hostname: %s\n", hostname);
        
        struct hostent* host = gethostbyname(hostname);
        if (host != NULL)
        {
            printf("    Local IP Addresses:\n");
            for (int i = 0; host->h_addr_list[i] != NULL; i++)
            {
                struct in_addr addr;
                memcpy(&addr, host->h_addr_list[i], sizeof(struct in_addr));
                printf("    %s\n", inet_ntoa(addr));
            }
        }
    }

    WSACleanup();
}

void print_NetworkAdapters()
{
    PIP_ADAPTER_INFO pAdapterInfo;
    ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
    
    pAdapterInfo = (IP_ADAPTER_INFO*)malloc(ulOutBufLen);
    if (pAdapterInfo == NULL)
    {
        printf("    Network Adapters: Memory allocation failed\n");
        return;
    }

    if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW)
    {
        free(pAdapterInfo);
        pAdapterInfo = (IP_ADAPTER_INFO*)malloc(ulOutBufLen);
        if (pAdapterInfo == NULL)
        {
            printf("    Network Adapters: Memory allocation failed\n");
            return;
        }
    }

    if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == NO_ERROR)
    {
        PIP_ADAPTER_INFO pAdapter = pAdapterInfo;
        while (pAdapter)
        {
            printf("  Adapter: %s\n", pAdapter->Description);
            printf("    IP: %s\n", pAdapter->IpAddressList.IpAddress.String);
            
            printf("    MAC: ");
            for (UINT i = 0; i < pAdapter->AddressLength; i++)
            {
                if (i == (pAdapter->AddressLength - 1))
                    printf("%.2X", (int)pAdapter->Address[i]);
                else
                    printf("%.2X:", (int)pAdapter->Address[i]);
            }
            printf("\n");
            
            pAdapter = pAdapter->Next;
        }
    }
    else
    {
        printf("    Network Adapters: GetAdaptersInfo failed\n");
    }

    if (pAdapterInfo)
        free(pAdapterInfo);
}

void WindowsInfo() {
    SYSTEM_INFO systemInfo;
    OSVERSIONINFOEX osVersionInfo;

    // Получаем информацию о системе
    GetSystemInfo(&systemInfo);
    ZeroMemory(&osVersionInfo, sizeof(OSVERSIONINFOEX));
    osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

    // Используем GetVersionEx для получения версии ОС
    if (!GetVersionEx((OSVERSIONINFO*)&osVersionInfo)) {
        printf("Failed to get OS version\n");
        return;
    }

    printf("    Processor (Windows) Architecture: ");
    switch (systemInfo.wProcessorArchitecture) {
        case PROCESSOR_ARCHITECTURE_AMD64:
            printf("  x64 (64-bit)\n");
            break;
        case PROCESSOR_ARCHITECTURE_INTEL:
            printf("  x86 (32-bit)\n");
            break;
        case PROCESSOR_ARCHITECTURE_ARM:
            printf("  ARM (32-bit)\n");
            break;
        #ifdef PROCESSOR_ARCHITECTURE_ARM64
        case PROCESSOR_ARCHITECTURE_ARM64:
            printf("  ARM64 (64-bit)\n");
            break;
        #endif
        case PROCESSOR_ARCHITECTURE_IA64:
            printf("Intel Itanium\n");
            break;
        default:
            printf("Unknown (Code: %d)\n", systemInfo.wProcessorArchitecture);
    }

    printf("    Number of Processors:               %u\n", systemInfo.dwNumberOfProcessors);
    printf("    Processor Level:                    %u\n", systemInfo.wProcessorLevel);
    printf("    Processor Revision:                 0x%04X\n", systemInfo.wProcessorRevision);
    printf("    Page Size:                          %u bytes\n", systemInfo.dwPageSize);

    // Информация о версии Windows
    printf("\n=== WINDOWS VERSION INFO ===\n");
    printf("    Version:                            %lu.%lu\n", osVersionInfo.dwMajorVersion, osVersionInfo.dwMinorVersion);
    printf("    Build Number:                       %u\n", (unsigned int)osVersionInfo.dwBuildNumber);

    const char* windowsName = " Unknown";
    if (osVersionInfo.dwMajorVersion == 10) {
        if (osVersionInfo.dwBuildNumber >= 22000) {
            windowsName = "                   Windows 11";
        } else {
            windowsName = "                   Windows 10";
        }
    } else if (osVersionInfo.dwMajorVersion == 6) {
        if (osVersionInfo.dwMinorVersion == 3) {
            windowsName = "                   Windows 8.1";
        } else if (osVersionInfo.dwMinorVersion == 2) {
            windowsName = "                   Windows 8";
        } else if (osVersionInfo.dwMinorVersion == 1) {
            windowsName = "                   Windows 7";
        } else if (osVersionInfo.dwMinorVersion == 0) {
            windowsName = "                   Windows Vista";
        }
    } else if (osVersionInfo.dwMajorVersion == 5) {
        if (osVersionInfo.dwMinorVersion == 2) {
            windowsName = "                   Windows XP 64-bit / Server 2003";
        } else if (osVersionInfo.dwMinorVersion == 1) {
            windowsName = "                   Windows XP";
        } else if (osVersionInfo.dwMinorVersion == 0) {
            windowsName = "                   Windows 2000";
        }
    }

    printf("    Windows Edition: %s\n", windowsName);

    // Service Pack информация
    if (strlen(osVersionInfo.szCSDVersion) > 0) {
        printf("    Service Pack: %s\n", osVersionInfo.szCSDVersion);
    }

    // Платформа
    printf("    Platform ID:                        %lu", osVersionInfo.dwPlatformId);
    if (osVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT) {
        printf(" (Windows NT)\n");
    } else {
        printf("\n");
    }

    // Дополнительная системная информация
    printf("\n=== ADDITIONAL SYSTEM INFO ===\n");

    // Проверяем, является ли система 64-битной
    BOOL isWow64 = FALSE;
    typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
    LPFN_ISWOW64PROCESS fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(
        GetModuleHandleA("kernel32"), "IsWow64Process");
    
    if (fnIsWow64Process != NULL) {
        fnIsWow64Process(GetCurrentProcess(), &isWow64);
        printf("    Is Running under WOW64:             %s\n", isWow64 ? "Yes" : "No");
    }

    // Получаем системные директории
    char systemDir[MAX_PATH];
    GetSystemDirectoryA(systemDir, MAX_PATH);
    printf("    System Directory:                   %s\n", systemDir);

    char windowsDir[MAX_PATH];
    GetWindowsDirectoryA(windowsDir, MAX_PATH);
    printf("    Windows Directory:                  %s\n", windowsDir);

    // Время работы системы
    DWORD uptime = GetTickCount();
    printf("    System Uptime:                      %u days, %u hours, %u minutes\n", 
           uptime / (1000 * 60 * 60 * 24),
           (uptime / (1000 * 60 * 60)) % 24,
           (uptime / (1000 * 60)) % 60);
}


void print_SystemInfo()
{
    system("cls");
    Logger::info("Displaying system information\n\n");
    SYSTEM_INFO siSysInfo;
    GetSystemInfo(&siSysInfo); 

    WindowsInfo();

    printf("\n\n=== System Information ===\n");
    printf("    OEM ID:                             %u\n", siSysInfo.dwOemId);
    printf("    Number of processors:               %u\n", siSysInfo.dwNumberOfProcessors); 
    printf("    Page size:                          %u\n", siSysInfo.dwPageSize); 
    printf("    Processor type:                     %u\n", siSysInfo.dwProcessorType); 
    printf("    Minimum application address:        %p\n", siSysInfo.lpMinimumApplicationAddress); 
    printf("    Maximum application address:        %p\n", siSysInfo.lpMaximumApplicationAddress); 
    printf("    Active processor mask:              %u\n", siSysInfo.dwActiveProcessorMask); 
    
    printf("\n\n=== User Information ===\n");
    print_Username();
    print_ComputerName();
    
    printf("\n\n=== Hardware ID ===\n");
    print_HWID();
    
    printf("\n\n=== Network Information ===\n");
    print_ExternalIP();
    print_IPAddresses();
    
    printf("\n\n=== Network Adapters ===\n");
    print_NetworkAdapters();
    _getch();
}