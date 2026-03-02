// DNS and SSL certificate cleanup (CRL, OCSP, etc.)
#include "iostream"
#include "string"
#include "vector"

#include "conio.h"
#include "windows.h"
#include <cstdio>
#include <limits>

#include "../../Logger.h"
#include "../../h_file/main.h"

using namespace std;


int clear_dns_ssl_lite_mode() {
	Logger::info("Starting DNS and SSL cleanup");
	system("cls");
	cout << "========================================" << endl;
	cout << "   DNS and SSL Cleanup" << endl;
	cout << "========================================" << endl << endl;

	cout << "Cleaning DNS and SSL certificates..." << endl << endl;

	cout << "Cleaning DNS cache..." << endl;
	system("ipconfig /flushdns");

	cout << "Cleaning Winsock..." << endl;
	system("netsh winsock reset");

	cout << "Cleaning TCP/IP..." << endl;
	system("netsh int ip reset");

	cout << "Cleaning SSL certificates..." << endl;
	system("certutil -urlcache * delete");

	cout << "Cleaning SSL certificates..." << endl;
	system("certutil -urlcache * delete");

	cout << "done" << endl;
	// cout << endl << "Reboot the system to apply the changes?\ny/N: " << endl;
	// string reboot;
	// cin >> reboot;
	// if (reboot == "y" || reboot == "Y") {
	// 	cout << "Reboot....";
	// 	Sleep(1000);
	// 	system("shutdown /r /t 5");
	// }
	// return 0;
}

int clear_dns_ssl_forced_mode() {
	string reboot;
	Logger::info("Starting DNS and SSL cleanup");
	system("cls");
	cout << "========================================" << endl;
	cout << "   DNS and SSL Cleanup ( forced mode )" << endl;
	cout << "========================================" << endl << endl;
	
	cout << "reset Winsock and TCP/IP..." << endl;
	system("netsh winsock reset catalog");

	cout << "delete SSL certificates..." << endl;
	system("certutil -urlcache * delete");

	cout << "Reset the SSL cache with force deletion" << endl;
	system("certutil -setreg chain\\ChainCacheSerialization\\AClientAuthFlags 0x00000001");
	system("certutil -setreg chain\\ChainCacheSerialization\\AServerAuthFlags 0x00000001");
	system("certutil -setreg chain\\ChainCacheSerialization\\ClientAuthFlags 0x00000001");
	system("certutil -setreg chain\\ChainCacheSerialization\\ServerAuthFlags 0x00000001");

	cout << "Force Certificate Cache Clearing" << endl;
	system("certutil -urlcache * delete");
	system("certutil -deleteatl");

	cout << "Resetting proxy settings" << endl;
	system("netsh winhttp reset proxy");

	cout << "reset TCP/IP..." << endl;
	system("netsh int ip reset reset.log");

	cout << "release and renew IP address..." << endl;
	system("ipconfig /release");
	system("ipconfig /renew");

	cout << "done" << endl;
	// cout << endl << "Reboot the system to apply the changes? (y/N or Enter to exit): ";
	// int key = _getch();

	// if (key == 'y' || key == 'Y') {
	// 		cout << "Rebooting..." << endl;
	// 		Sleep(1000);
	// 		system("shutdown /r /t 5");
	// }
	// return 0;
}

int showRootSSLInfo() {
	system("cls");
	system("certutil -store Root");
	_getch();
}
int showRootSSLInfo_tt() {
	system("cls");

	// Get certutil -store Root output
	FILE* pipe = _popen("certutil -store Root", "r");
	if (!pipe) {
		cout << "Failed to start certutil." << endl;
		_getch();
		return 0;
	}

	vector<string> lines;
	vector<size_t> certStarts;
	char buffer[1024];

	while (fgets(buffer, sizeof(buffer), pipe)) {
		string line(buffer);
		// Remove newline characters at the end
		if (!line.empty() && (line.back() == '\n' || line.back() == '\r')) {
			while (!line.empty() && (line.back() == '\n' || line.back() == '\r')) {
				line.pop_back();
			}
		}
		// Store the line
		lines.push_back(line);

		// Look for certificate boundaries
		if (line.rfind("================ Certificate ", 0) == 0) {
			certStarts.push_back(lines.size() - 1);
		}
	}
	_pclose(pipe);

	if (certStarts.empty()) {
		cout << "Certificates not found in Root store." << endl;
		_getch();
		return 0;
	}

	while (true) {
		system("cls");
		cout << "Found certificates: " << certStarts.size() << endl;
		cout << "==============================" << endl;
		for (size_t i = 0; i < certStarts.size(); ++i) {
			cout << "certificate {" << (i + 1) << "}" << endl;
		}
		cout << "==============================" << endl;
		cout << "Enter certificate number to view (0 - exit): ";

		int choice;
		if (!(cin >> choice)) {
			cin.clear();
			cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			continue;
		}

		if (choice == 0) {
			break;
		}
		if (choice < 0 || static_cast<size_t>(choice) > certStarts.size()) {
			continue;
		}

		size_t idx = static_cast<size_t>(choice - 1);
		size_t start = certStarts[idx];
		size_t end = lines.size();
		if (idx + 1 < certStarts.size()) {
			end = certStarts[idx + 1];
		}

		system("cls");
		for (size_t i = start; i < end; ++i) {
			cout << lines[i] << endl;
		}
		cout << endl << "Press any key to return to certificate list..." << endl;
		_getch();
	}

	return 0;
}

int showUserSSLInfo() {}

int showSSLInfo() {
	// ROOT and USER SSL-information
	cout << "ncurses, ROOT and USER SSL Information" << endl;
	cout << "========================================" << endl;
	cout << "   SSL Information" << endl;
	cout << "========================================" << endl << endl;

	vector<string> sslInfo = {
		"ROOT",
		"USER"
	};
	int selectedIndex = 0;
	bool running = true;

	while (running) {
		system("cls");
		cout << "'q' to quit" << endl;
		cout << "Use Up/Down arrows to navigate, Enter to select" << endl << endl;
		
		for (int i = 0; i < sslInfo.size(); i++) {
			if (i == selectedIndex) {
				cout << "> " << sslInfo[i] << endl;
			} else {
				cout << "  	" << sslInfo[i] << endl;
			}
		}
		int key = _getch();
		
		if (g_ctrlCPressed)
		{
				g_ctrlCPressed = FALSE;
				running = false;
				continue;
		}
		if (key == 224) 
		{
			
			key = _getch();
			switch (key) {
				case 72: // Up arrow
					selectedIndex = (selectedIndex - 1 + sslInfo.size()) % sslInfo.size();
					break;
				case 80: // Down arrow
					selectedIndex = (selectedIndex + 1) % sslInfo.size();
					break;
			}
		}
		else if (key == 13)
		{ // Enter
			switch (selectedIndex) 
			{
				case 0: // ROOT
					showRootSSLInfo();
					break;
				case 1: // USER
					showUserSSLInfo();
					break;
			}
		}
		else if (key == 'q' || key == 'Q')
		{
			system("cls");
			running = false;
		}
	}

}