#pragma once

#include <Windows.h>

namespace registry {

	
	/// <summary>
	/// Check that a given registry key exists AND a given value exists AND the value contains a certain string.
	/// Intended for REG_SZ value types only. Note that this assumes C-strings and not unicode strings
	/// </summary>
	/// <param name="topLevelKey">The top-level key, e.g. HKEY_LOCAL_MACHINE</param>
	/// <param name="subKey">The rest of the subkey, e.g. "HARDWARE\DEVICEMAP\Scsi\Scsi Port 0</param>
	/// <param name="valueName">The name of the value to read, e.g. "Interrupt"</param>
	/// <param name="toMatch">Check if the value contains this value</param>
	/// <returns>Boolean indicating if the key exists and contains the given substring</returns>
	bool keyValueExistsAndContainsStr(HKEY topLevelKey, LPCSTR subKey, LPCSTR valueName, LPCSTR toMatch);


	/// <summary>
	/// Check if a given registry key/subkey exists. 
	/// </summary>
	/// <param name="topLevelKey">The top-level key, e.g. HKEY_CURRENT_USER</param>
	/// <param name="subKey">The rest of the subkey, e.g. "Environment"</param>
	/// <returns>Boolean indicating if the key exists</returns>
	bool keyExists(HKEY topLevelKey, LPCSTR subKey);
}

namespace filesystem {

	/// <summary>
	/// Check if a file exists
	/// </summary>
	/// <param name="fileAbsolutePath">The absolute path to the file, e.g. C:\windows\system32\cmd.exe</param>
	/// <returns></returns>
	bool fileExists(LPCSTR fileAbsolutePath);
}

namespace hardware {

	/// <summary>
	/// Pass in a pointer to 3 bytes and check if the device has a MAC address beginning with the given bytes
	/// </summary>
	/// <param name="macVendorBytes">The bytes to look for in MAC addresses</param>
	/// <returns></returns>
	bool macVendorMatches(char* macVendorBytes);
}