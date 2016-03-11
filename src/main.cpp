#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <chrono>
#include <windows.h>
#include <process.h>
#include <Tlhelp32.h>
#include <winbase.h>

using namespace std;

#define GUEST_FILE "guest.txt"
#define PAUSE_TIME 30

/*
	Kills a Windows process by name
*/
void killProcessByName(const char *filename) {
    HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, NULL);
    PROCESSENTRY32 pEntry;
    pEntry.dwSize = sizeof (pEntry);
    BOOL hRes = Process32First(hSnapShot, &pEntry);
    while (hRes) {
        if (strcmp(pEntry.szExeFile, filename) == 0) {
            HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, 0,
                                          (DWORD) pEntry.th32ProcessID);
            if (hProcess != NULL) {
                TerminateProcess(hProcess, 9);
                CloseHandle(hProcess);
            }
        }
        hRes = Process32Next(hSnapShot, &pEntry);
    }
    CloseHandle(hSnapShot);
}

/*
	Removes colour formatting from LFS colour encoded strings
	TODO: Fix hostname stripping which have ^ symbol without colour codes
*/
string strip_chars(string str) {
	bool rem = false;
	string newstr;
	// Loop through every character looking for ^
	for (unsigned int i = 0; i < str.length(); i++) {
		if (str[i] == '^') rem = true;
		else if (rem == true) rem = false;
		else newstr += str[i];
	}
	return newstr;
}

/*
	Parses the host name the LFS client is connected to
*/
string get_host_name() {
	ifstream guestFile (GUEST_FILE, ifstream::in);
	string line;
	string host;
	
	if (guestFile.is_open()) {
		// Read through lines in text file
		// Find line with "joined="
		// Parse host name after equals symbol
		// Strip LFS colour codes
		while (getline(guestFile, line)) {
			if (line.find("joined=") != string::npos) {
				host = strip_chars(line.substr(7));
				break;
			}
		}
		guestFile.close();
	}
	
	return host;
}

/*
	Returns true if the guest file still exists
*/
bool guest_file_exists() {
	ifstream guestFile (GUEST_FILE, ifstream::in);
	// Check if file is opened
	// If true, we know the file exists
	if (guestFile.is_open()) {
		guestFile.close();
		return true;
	}
	return false;
}

int main(int argc, char **argv) {
	string host = get_host_name();
	
	// Check if host name was parsed
	if (!host.empty()) {
		cout << "LFS will automatically reconnect to " << host << endl;
		
		// Continuously loop checking for the guest file
		// Once the guest file disappears we know LFS lost connection or crashed
		while (1) {
			if (!guest_file_exists()) {
				cout << "Couldn't open \"guest.txt\" file. Assuming lost connection" << endl;
				cout << "Killing LFS" << endl;
				killProcessByName("LFS.exe");
				
				// Wait for a second before relaunching LFS
				this_thread::sleep_for(chrono::seconds(1));
				
				cout << "Launching LFS" << endl;
				system(("start LFS.exe /join=" + host).c_str());
				cout << "LFS relaunched" << endl;
			}
			
			// Pause the loop for 30 seconds before repeating
			this_thread::sleep_for(chrono::seconds(PAUSE_TIME));
		}
	}	
	else {
		cout << "Error opening \"guest.txt\" file. Is LFS running and conncted to a host?" << endl;
		system("pause");
	}
	return 0;
}
