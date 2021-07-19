#pragma once

#include <string>

namespace decryption {

	/// <summary>
	/// decryptKey takes a char buffer, and uses a string (char*) key to iteratively decrypt it
	/// </summary>
	/// <param name="keyStr">pointer to a char buffer containing the decryption key</param>
	/// <param name="sc">pointer to a char buffer containing the shellcode to decrypt</param>
	/// <param name="scLen">the length of the shellcode buffer</param>
	/// <returns>a pointer to a new buffer that contains the decrypted shellcode. Must be cleaned up with delete[].</returns>
	unsigned char* decryptKey(unsigned char* keyStr, unsigned char* sc, size_t scLen);

	/// <summary>
	/// decyptXOR performs a bytewise XOR decryption.
	/// </summary>
	/// <param name="byteKey">the byte key - will be XOR'ed with each byte in the shellcode buffer</param>
	/// <param name="sc">a pointer to the buffer containing the shellcode to decrypt</param>
	/// <param name="scLen">the length of the shellcode buffer</param>
	/// <returns>a pointer to a new buffer that contains the decrypted shellcode. Must be cleaned up with delete[]</returns>
	unsigned char* decryptXOR(unsigned char byteKey, unsigned char* sc, size_t scLen);
}
