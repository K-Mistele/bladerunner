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
	/// <returns></returns>
	bool keyValueExistsAndContainsStr(HKEY topLevelKey, LPCSTR subKey, LPCSTR valueName, LPCSTR toMatch);
}