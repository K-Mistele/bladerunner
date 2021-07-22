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