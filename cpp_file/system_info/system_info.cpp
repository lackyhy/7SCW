#include <windows.h>
#include <stdio.h>
#include <iphlpapi.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <wininet.h>

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
        printf("Username: %s\n", username);
    }
    else
    {
        printf("Username: Unable to retrieve\n");
    }
}

void print_ComputerName()
{
    char computername[256];
    DWORD size = sizeof(computername);
    if (GetComputerNameA(computername, &size))
    {
        printf("Computer Name: %s\n", computername);
    }
    else
    {
        printf("Computer Name: Unable to retrieve\n");
    }
}

void print_HWID()
{
    DWORD dwSerial;
    if (GetVolumeInformationA("C:\\", NULL, 0, &dwSerial, NULL, NULL, NULL, 0))
    {
        printf("HWID: %lu\n", dwSerial);
    }
    else
    {
        printf("HWID: Unable to retrieve\n");
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
                printf("External IP: %s\n", buffer);
            }
            else
            {
                printf("External IP: Unable to retrieve\n");
            }
            InternetCloseHandle(hConnect);
        }
        else
        {
            printf("External IP: Unable to connect\n");
        }
        InternetCloseHandle(hInternet);
    }
    else
    {
        printf("External IP: Internet unavailable\n");
    }
}

void print_IPAddresses()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("IP Address: WSAStartup failed\n");
        return;
    }

    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == 0)
    {
        printf("Hostname: %s\n", hostname);
        
        struct hostent* host = gethostbyname(hostname);
        if (host != NULL)
        {
            printf("Local IP Addresses:\n");
            for (int i = 0; host->h_addr_list[i] != NULL; i++)
            {
                struct in_addr addr;
                memcpy(&addr, host->h_addr_list[i], sizeof(struct in_addr));
                printf("  %s\n", inet_ntoa(addr));
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
        printf("Network Adapters: Memory allocation failed\n");
        return;
    }

    if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW)
    {
        free(pAdapterInfo);
        pAdapterInfo = (IP_ADAPTER_INFO*)malloc(ulOutBufLen);
        if (pAdapterInfo == NULL)
        {
            printf("Network Adapters: Memory allocation failed\n");
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
        printf("Network Adapters: GetAdaptersInfo failed\n");
    }

    if (pAdapterInfo)
        free(pAdapterInfo);
}

void print_SystemInfo()
{
    SYSTEM_INFO siSysInfo;
    GetSystemInfo(&siSysInfo); 

    printf("=== System Information ===\n");
    printf("OEM ID: %u\n", siSysInfo.dwOemId);
    printf("Number of processors: %u\n", siSysInfo.dwNumberOfProcessors); 
    printf("Page size: %u\n", siSysInfo.dwPageSize); 
    printf("Processor type: %u\n", siSysInfo.dwProcessorType); 
    printf("Minimum application address: %p\n", siSysInfo.lpMinimumApplicationAddress); 
    printf("Maximum application address: %p\n", siSysInfo.lpMaximumApplicationAddress); 
    printf("Active processor mask: %u\n", siSysInfo.dwActiveProcessorMask); 
    
    printf("\n=== User Information ===\n");
    print_Username();
    print_ComputerName();
    
    printf("\n=== Hardware ID ===\n");
    print_HWID();
    
    printf("\n=== Network Information ===\n");
    print_ExternalIP();
    print_IPAddresses();
    
    printf("\n=== Network Adapters ===\n");
    print_NetworkAdapters();
}