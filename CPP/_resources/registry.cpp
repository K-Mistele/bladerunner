#include "registry.h"
#include <Windows.h>
#include <cstring>
#include <string>
#include <iostream>

using std::cout;
using std::endl;
using std::string;

namespace registry {

	bool keyValueExistsAndContainsStr(HKEY topLevelKey, LPCSTR subKey, LPCSTR valueName, LPCSTR toMatch) {

		DWORD foundKeyType = 0; // THE TYPE OF THE KEY THAT IS ACTUALLY READ
		char* outData = new char[1024];
		outData[0] = 'A';
		outData[1023] = 'A';
		DWORD outDataSize = 1024;
		DWORD regStatusType = RRF_RT_REG_SZ | RRF_RT_REG_EXPAND_SZ;
		LSTATUS result = RegGetValueA(topLevelKey, subKey, valueName, regStatusType, &foundKeyType, outData, &outDataSize);
		if (result != ERROR_SUCCESS) {
			cout << "Failed to fetch registry value with status " << result << endl;
			return false;
		}
		// TODO: PROCESS AND SEARCH THROUGH, MAY REQUIRE ADDITIONAL SEARCHING
		bool dataFound = false;
		cout << "Read " << outDataSize << " bytes from reg value " << toMatch << endl;

		char* needle_buf = new char[outDataSize + 1];
		strncpy(needle_buf, outData, outDataSize);
		string haystack = string(needle_buf);
		string needle = string(toMatch);

		cout << "Haystack: " << haystack << endl;
		cout << "Needle: " << needle << endl;
		
		if (haystack.find(needle) != string::npos) {
			dataFound = true;
			cout << "Found string!" << endl;
		}
		
		delete[] outData;
		return dataFound;

	}
}