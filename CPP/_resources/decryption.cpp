#include "decryption.h"

namespace decryption {

	// PERFORM DECRYPTION WITH A KEY
	unsigned char* decryptKey(unsigned char* keyStr, size_t keyStrLen, unsigned char* sc, size_t scLen) {

		// ALLOCATE SPACE FOR DECRYPTED SHELLCODE
		unsigned char* decrData = new unsigned char[scLen + 1]; // ALLOCATE + 1 FOR NULL BYTE

		// i IS THE INDEX IN THE SC
		// j IS THE INDEX IN THE KEY
		size_t j = 0;
		for (size_t i = 0; i < scLen; i++) {

			// IF WE'RE AT THE END OF THE KEY, GO BACK TO THE BEGINNING
			if (j == keyStrLen) {
				j = 0;
			}
			decrData[i] = sc[i] ^ keyStr[j];
			j++;
		}
		decrData[scLen] = '\x00';
		return decrData;
	}

	// PERFORM BYTEWISE XOR DECRYPTION
	unsigned char* decryptXOR(unsigned char byteKey, unsigned char* sc, size_t scLen) {
		

		// ALLOCATE SPACE FOR THE DECRYPTED SHELLCODE
		unsigned char* decrData = new unsigned char[scLen + 1];

		for (size_t i = 0; i < scLen; i++) {
			decrData[i] = sc[i] ^ byteKey;
		}

		decrData[scLen] = '\x00';
		return decrData;
	}
}