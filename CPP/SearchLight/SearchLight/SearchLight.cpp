#include <iostream>
#include <Windows.h>
#include <string>
#include <vector>

using std::string;
using std::vector;
using std::cout;
using std::endl;

// NTDLL FUNCTIONS THAT PRODUCE FALSE POSITIVES AND SO SHOULD NOT BE CONSIDERED AS HOOKED
vector<string> falsePositives = {
	"NtGetTickCount", 
	"NtQuerySystemTime", 
	"NtdllDefWindowProc_A", 
	"NtdllDefWindowProc_W", 
	"NtdllDialogWndProc_A", 
	"NtdllDialogWndProc_W", 
	"ZwQuerySystemTime"
};

// WIN API CALLS TO CHECK FOR HOOKING, UPDATE THIS WITH THE FUNCTIONS YOU WANT TO MESS WITH
vector<string> winAPICalls = {
	"VirtualAlloc",
	"VirtualAllocEx",
	"VirtualProtect",
	"VirtualProtectEx",
	"CreateProcessA",
	"OpenProcess",
	"WriteProcessMemory",
	"CreateRemoteThread",
	"CloseHandle"

};

int main()
{

	// CHECK FOR HOOKED SYSTEM CALLS IN NTDLL
	PDWORD functionAddress = (PDWORD)0;

	// Get ntdll base address
	HMODULE libraryBase = LoadLibraryA("ntdll");

	PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)libraryBase;
	PIMAGE_NT_HEADERS imageNTHeaders = (PIMAGE_NT_HEADERS)((DWORD_PTR)libraryBase + dosHeader->e_lfanew);

	// Locate export address table
	DWORD_PTR exportDirectoryRVA = imageNTHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
	PIMAGE_EXPORT_DIRECTORY imageExportDirectory = (PIMAGE_EXPORT_DIRECTORY)((DWORD_PTR)libraryBase + exportDirectoryRVA);

	// Offsets to list of exported functions and their names
	PDWORD addresOfFunctionsRVA = (PDWORD)((DWORD_PTR)libraryBase + imageExportDirectory->AddressOfFunctions);
	PDWORD addressOfNamesRVA = (PDWORD)((DWORD_PTR)libraryBase + imageExportDirectory->AddressOfNames);
	PWORD addressOfNameOrdinalsRVA = (PWORD)((DWORD_PTR)libraryBase + imageExportDirectory->AddressOfNameOrdinals);

	int hookedSyscallCount = 0;
	// Iterate through exported functions of ntdll
	for (DWORD i = 0; i < imageExportDirectory->NumberOfNames; i++)
	{
		// Resolve exported function name
		DWORD functionNameRVA = addressOfNamesRVA[i];
		DWORD_PTR functionNameVA = (DWORD_PTR)libraryBase + functionNameRVA;
		char* functionName = (char*)functionNameVA;

		// Resolve exported function address
		DWORD_PTR functionAddressRVA = 0;
		functionAddressRVA = addresOfFunctionsRVA[addressOfNameOrdinalsRVA[i]];
		functionAddress = (PDWORD)((DWORD_PTR)libraryBase + functionAddressRVA);

		// Syscall stubs start with these bytes
		char syscallPrologue[4] = { 0x4c, 0x8b, 0xd1, 0xb8 };

		// Only interested in Nt|Zw functions
		
		if (strncmp(functionName, (char*)"Nt", 2) == 0 || strncmp(functionName, (char*)"Zw", 2) == 0)
		{
			// Check if the first 4 instructions of the exported function are the same as the sycall's prologue
			if (memcmp(functionAddress, syscallPrologue, 4) != 0) {

				// CHECK TO MAKE SURE IT ISN'T IN THE LIST OF FALSE POSITIVE FUNCTIONS
				bool shouldSkip = false;
				for (size_t j = 0; j < falsePositives.size(); j++) {
					if (strcmp(falsePositives.at(j).c_str(), functionName) == 0) {
						cout << "Skipping a known false positive " << functionName << endl;
						shouldSkip = true;
					}
				}
				if (!shouldSkip) {
					printf("Hooked syscall in NTDLL: %s : %p\n", functionName, functionAddress);
					hookedSyscallCount += 1;
				}
			}
		}
	}

	cout << "Found " << hookedSyscallCount << " hooked syscalls!" << endl;

	// CHECK FOR HOOKED API CALLS
	HINSTANCE ntdll = LoadLibraryA("kernel32.dll");
	for (size_t k = 0; k < winAPICalls.size(); k++) {

		string apiCallName = winAPICalls.at(k);
		void* apiCallAddress = nullptr;
		apiCallAddress = GetProcAddress(ntdll, apiCallName.c_str());
		if (!apiCallAddress) {
			cout << "Unable to find " << apiCallName << " in NTDLL" << endl;
		}
		cout << apiCallName << " is at " << apiCallAddress << endl;

	}

	return 0;
}