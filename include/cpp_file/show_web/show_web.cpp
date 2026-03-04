#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif

#include <iostream>
#include <vector>
#include <string>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <wininet.h>
#include <conio.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "wininet.lib")

#ifndef NO_ERROR
#define NO_ERROR 0L
#endif
#ifndef ERROR_BUFFER_OVERFLOW
#define ERROR_BUFFER_OVERFLOW 111L
#endif

using namespace std;

string get_external_ip(const string& url) {
    HINTERNET hInternet = InternetOpenA("IP-Checker", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) return "Error";

    HINTERNET hConnect = InternetOpenUrlA(hInternet, url.c_str(), NULL, 0, INTERNET_FLAG_RELOAD, 0);
    if (!hConnect) {
        InternetCloseHandle(hInternet);
        return "Not Available";
    }

    char buffer[128];
    DWORD bytesRead;
    string result = "";
    if (InternetReadFile(hConnect, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead > 0) {
        buffer[bytesRead] = '\0';
        result = buffer;
    }

    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);
    return result;
}

void show_web() {
    system("cls");
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) return;

    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) != SOCKET_ERROR) {
        struct hostent* host = gethostbyname(hostname);
        if (host != nullptr) {
            cout << " [Local IP Addresses]" << endl;
            for (int i = 0; host->h_addr_list[i] != 0; ++i) {
                struct in_addr addr;
                memcpy(&addr, host->h_addr_list[i], sizeof(struct in_addr));
                cout << "	Local: " << inet_ntoa(addr) << endl;
            }
        }
    }

    cout << "\n [Router Web Interface]" << endl;
    ULONG outBufLen = sizeof(IP_ADAPTER_INFO);
    PIP_ADAPTER_INFO pAdapterInfo = (IP_ADAPTER_INFO*)malloc(sizeof(IP_ADAPTER_INFO));
    DWORD dwRetVal = GetAdaptersInfo(pAdapterInfo, &outBufLen);
    if (dwRetVal == ERROR_BUFFER_OVERFLOW) {
        free(pAdapterInfo);
        pAdapterInfo = (IP_ADAPTER_INFO*)malloc(outBufLen);
        dwRetVal = GetAdaptersInfo(pAdapterInfo, &outBufLen);
    }

    if (dwRetVal == NO_ERROR) {
        PIP_ADAPTER_INFO pAdapter = pAdapterInfo;
        while (pAdapter) {
            string gateway = pAdapter->GatewayList.IpAddress.String;
            if (gateway != "0.0.0.0" && gateway.length() > 0) {
                cout << "	Router IP: " << gateway << endl;
                cout << "	Web Page: http://" << gateway << endl;
            }
            pAdapter = pAdapter->Next;
        }
    }
    if (pAdapterInfo) free(pAdapterInfo);
    WSACleanup();
}

void show_external_info() {
    cout << "\n [Public Network Info]" << endl;
    cout << "	External IPv4: " << get_external_ip("http://api.ipify.org") << endl;
    cout << "	External IPv6: " << get_external_ip("http://api6.ipify.org") << endl;
}

void show_web_all() {
    show_web();
    show_external_info();
    cout << "\nPress any key to exit..." << endl;
    _getch();
}