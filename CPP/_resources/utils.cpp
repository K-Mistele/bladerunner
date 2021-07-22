
#include "utils.h"
#include <Windows.h>
#include <cstring>
#include <string>
#include <iostream>

using std::cout;
using std::endl;
using std::string;

///////////////////////////////////////////////////////////////////////////////
// NOTE THAT SOME DETECTIONS ARE FROM https://github.com/dsnezhkov/pufferfish
///////////////////////////////////////////////////////////////////////////////

namespace registry {

	bool keyValueExistsAndContainsStr(HKEY topLevelKey, LPCSTR subKey, LPCSTR valueName, LPCSTR toMatch) {

		DWORD foundKeyType = 0; // THE TYPE OF THE KEY THAT IS ACTUALLY READ
		char* outData = new char[10240];
		DWORD outDataSize = 10240; // BOTH AN IN AND AN OUT TYPE
		DWORD regStatusType = RRF_RT_REG_SZ | RRF_RT_REG_EXPAND_SZ | RRF_RT_REG_MULTI_SZ;
		LSTATUS result = RegGetValueA(topLevelKey, subKey, valueName, regStatusType, &foundKeyType, outData, &outDataSize);
		if (result != ERROR_SUCCESS) {
			return false;
		}
		// TODO: PROCESS AND SEARCH THROUGH, MAY REQUIRE ADDITIONAL SEARCHING
		bool dataFound = false;
		size_t i = 0; // INDEX IN REGISTRY VALUE DATA
		size_t j = 0; // REGISTRY IN SEARCH TERM
		char* haystack = outData; // THIS IS A RENAMING, DO NOT DELETE][] THIS OR YOU WILL GET A DOUBLE-FREE ERROR
		char* needle = (char*) toMatch;
		size_t haystackLen = outDataSize;
		size_t needleLen = strlen(needle);

		// NAIVE STRING MATCH ALGORITHM THAT IGNORES NULL TERMINATORS
		for (i = 0; i < outDataSize; i++) {
			
			while (i < haystackLen && j < needleLen && (i + j) < haystackLen && haystack[i + j] == needle[j]) {
				if (j == needleLen - 1) {
					cout << "match found at " << i << ", " << j << endl;
					dataFound = true;
					break;
				}
				j++;
			}
			if (dataFound) break;
			j = 0;

		}
		
		delete[] outData;
		return dataFound;

	}

	bool keyExists(HKEY topLevelKey, LPCSTR subKey) {

		DWORD foundKeyType = 0;
		HKEY keyHandle;
		LSTATUS result = RegOpenKeyExA(topLevelKey, subKey, 0, KEY_READ, &keyHandle);
		RegCloseKey(keyHandle);
		return result == ERROR_SUCCESS;
	}
}

namespace filesystem {

	bool fileExists(LPCSTR fileAbsolutePath) {
		return GetFileAttributesA(fileAbsolutePath) != INVALID_FILE_ATTRIBUTES;
	}
}

namespace hardware {
	
}