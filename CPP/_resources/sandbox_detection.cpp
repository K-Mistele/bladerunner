#include "sandbox_detection.h"
#include "Windows.h"
#include <iostream>
#include <string>
#include <cstring>
#include <Windows.h>
#include <WinDNS.h>



using std::string;
using std::cout;
using std::endl;

namespace sandboxDetection {

	// INTERNAL DATA AND METHODS
	namespace internal {

		bool debugMode = false;

		void debug(char* msg) {
			cout << "[+] " << msg << endl;
		}

		void debug(string msg) {
			cout << "[+] " << msg << endl;
		}
	}

	// TURN ON DEBUGGING
	void enableDebugMode() {
		internal::debugMode = true;
		internal::debug("Sandbox detection debugmode enabled");
	}

	void requireJoinedToDomain(char* domain) {

		size_t buffSize = 100;
		LPWSTR currentDomainWideStr = new WCHAR[buffSize];
		DWORD len = GetEnvironmentVariableW(L"USERDOMAIN", currentDomainWideStr, buffSize);
		if (len == 0) {
			internal::debug("Failed to get $USERDOMAIN");
			exit(0);
		}

		char* currentDomainStr = new char[buffSize];
		char defaultChar = '?';
		BOOL flagIsUsed = FALSE;
		
		// CONVERT WIDECHAR TO MULTIBYTE
		int chars = WideCharToMultiByte(CP_ACP, 0, currentDomainWideStr, buffSize, currentDomainStr, buffSize, &defaultChar, &flagIsUsed);
		if (chars == 0) {
			internal::debug("Failed to convert $USERDOMAIN to an acceptable string format, test results inconclusive");
			exit(0);
		}
		internal::debug("Grabbed domain name!");
		internal::debug(currentDomainStr);
		

		// CHECK TO SEE IF THEY MATCHED
		if (strcmp(domain, currentDomainStr) != 0) {
			internal::debug("Domain does not match, exiting");
			exit(0);
		}
		internal::debug("domains match!");

		// FREE MEMORY 
		delete[] currentDomainStr;
		delete[] currentDomainWideStr;
	}

	void requireResolvableDomainName(char* domain) {
		/*
		DNS_STATUS status = DnsQuery_A(
			domain,
			DNS_TYPE_A,
			DNS_QUERY_NO_HOSTS_FILE | DNS_QUERY_NO_MULTICAST | DNS_QUERY_STANDARD,
			NULL,
			NULL,
			NULL
		);
		cout << "[+] Status: " << "good" << endl;
		*/
		internal::debug("requireResolvableDomainName not implemented due to broken windns.h");
	}

	void requireNotWine() {

		//https://github.com/a0rtega/pafish/blob/master/pafish/wine.c
		HMODULE k32;
		k32 = GetModuleHandle(L"kernel32.dll");
		if (k32 != NULL) {
			if (GetProcAddress(k32, "wine_get_unix_file_name") != NULL) {
				internal::debug("Running in wine!");
				exit(0);
			}
			else {
				//return FALSE;
				internal::debug("Wine check passed");
			}
		}
		else {
			internal::debug("Failed to load kernel32, running in wine!");
			exit(0);
		}
	}
}