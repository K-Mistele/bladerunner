#include "sandbox_detection.h"
#include "Windows.h"
#include <iostream>
#include <string>
#include <cstring>
#include <Windows.h>
#include <WinDNS.h>
#include "utils.h"
#include <TlHelp32.h>
#include <intrin.h>



using std::string;
using std::cout;
using std::endl;


///////////////////////////////////////////////////////////////////////////////
// NOTE THAT MOST DETECTIONS ARE FROM https://github.com/dsnezhkov/pufferfish
///////////////////////////////////////////////////////////////////////////////

namespace sandboxDetection {

	// INTERNAL DATA AND METHODS
	namespace internal {

		bool debugMode = false;

		void debug(char* msg) {
			cout << "[+] " << msg << endl;
		}

		void debug(string msg) {
			cout << "[+] " << msg << endl;
		}


	}

	// TURN ON DEBUGGING
	void enableDebugMode() {
		internal::debugMode = true;
		internal::debug("Sandbox detection debugmode enabled");
	}

	void requireJoinedToDomain(char* domain) {

		size_t buffSize = 100;
		LPWSTR currentDomainWideStr = new WCHAR[buffSize];
		DWORD len = GetEnvironmentVariableW(L"USERDOMAIN", currentDomainWideStr, buffSize);
		if (len == 0) {
			internal::debug("Failed to get $USERDOMAIN");
			exit(0);
		}

		char* currentDomainStr = new char[buffSize];
		char defaultChar = '?';
		BOOL flagIsUsed = FALSE;
		
		// CONVERT WIDECHAR TO MULTIBYTE
		int chars = WideCharToMultiByte(CP_ACP, 0, currentDomainWideStr, buffSize, currentDomainStr, buffSize, &defaultChar, &flagIsUsed);
		if (chars == 0) {
			internal::debug("Failed to convert $USERDOMAIN to an acceptable string format, test results inconclusive");
			exit(0);
		}
		

		// CHECK TO SEE IF THEY MATCHED
		if (strcmp(domain, currentDomainStr) != 0) {
			internal::debug("Domain does not match, exiting");
			exit(0);
		}

		// FREE MEMORY 
		delete[] currentDomainStr;
		delete[] currentDomainWideStr;

		internal::debug("Domain name check passed");
	}

	void requireResolvableDomainName(char* domain) {
		/*
		DNS_STATUS status = DnsQuery_A(
			domain,
			DNS_TYPE_A,
			DNS_QUERY_NO_HOSTS_FILE | DNS_QUERY_NO_MULTICAST | DNS_QUERY_STANDARD,
			NULL,
			NULL,
			NULL
		);
		cout << "[+] Status: " << "good" << endl;
		*/
		internal::debug("requireResolvableDomainName not implemented due to broken windns.h");
	}

	void requireNotWine() {

		//https://github.com/a0rtega/pafish/blob/master/pafish/wine.c
		HMODULE k32;
		k32 = GetModuleHandle(L"kernel32.dll");
		if (k32 != NULL) {
			if (GetProcAddress(k32, "wine_get_unix_file_name") != NULL) {
				internal::debug("Running in wine!");
				exit(0);
			}
			else {
				//return FALSE;
				internal::debug("Wine check passed");
			}
		}
		else {
			internal::debug("Failed to load kernel32, running in wine!");
			exit(0);
		}
	}

	void requireNotVbox() {

		// TEST FOR A BUNCH OF COMMON VIRTUALBOX REG KEYS
		// https://github.com/dsnezhkov/pufferfish/blob/master/pafish/vbox.c

		bool isVbox = false;

		if (registry::keyValueExistsAndContainsStr(HKEY_LOCAL_MACHINE, "HARDWARE\\DEVICEMAP\\Scsi\\Scsi Port 0\\Scsi Bus 0\\Target Id 0\\Logical Unit Id 0", "Identifier", "VBOX")) {
			internal::debug("detected VirtualBox via hardware SCSI port-related registry key");
			isVbox = true;
		}

		if (registry::keyValueExistsAndContainsStr(HKEY_LOCAL_MACHINE, "HARDWARE\\Description\\System", "SystemBiosVersion", "VBOX")) {
			internal::debug("detected VirtualBox via BIOS-related registry key");
			isVbox = true;
		}

		if (registry::keyExists(HKEY_LOCAL_MACHINE, "SOFTWARE\\Oracle\\VirtualBox Guest Additions")) {
			internal::debug("detected VirtualBox via Guest Additions registry key");
			isVbox = true;
		}

		if (registry::keyValueExistsAndContainsStr(HKEY_LOCAL_MACHINE, "HARDWARE\\Description\\System", "VideoBiosVersion", "VIRTUALBOX")) {
			internal::debug("detected VirtualBox via Video Bios-related registry key");
			isVbox = true;
		}

		if (registry::keyExists(HKEY_LOCAL_MACHINE, "HARDWARE\\ACPI\\DSDT\\VBOX__") ||
			registry::keyExists(HKEY_LOCAL_MACHINE, "HARDWARE\\ACPI\\FADT\\VBOX__") || 
			registry::keyExists(HKEY_LOCAL_MACHINE, "HARDWARE\\ACPI\\RSDT\\VBOX__")) {
			internal::debug("detected VirtualBox via ACPI registry key");
			isVbox = true;
		}

		// CHECK FOR REGISTRY KEYS RELATED TO VIRTUAL BOX SERVICES
		string strs[5];
		strs[0] = "SYSTEM\\ControlSet001\\Services\\VBoxGuest";
		strs[1] = "SYSTEM\\ControlSet001\\Services\\VBoxMouse";
		strs[2] = "SYSTEM\\ControlSet001\\Services\\VBoxService";
		strs[3] = "SYSTEM\\ControlSet001\\Services\\VBoxSF";
		strs[4] = "SYSTEM\\ControlSet001\\Services\\VBoxVideo";
		for (size_t i = 0; i < 5; i++) {
			if (registry::keyExists(HKEY_LOCAL_MACHINE, strs[i].c_str())) {
				internal::debug("detected virtualbox by related service");
				isVbox = true;
				break;
			}
		}

		// DETECT BASED ON SYSTEM BIOS DATA
		if (registry::keyValueExistsAndContainsStr(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System", "SystemBiosDate", "06/23/99")) {
			internal::debug("detected Virtual Box by BIOS data");
			isVbox = true;
		}

		// DETECT BASED ON VIRTUALBOX DRIVER FILES
		string files[4];
		files[0] = "C:\\WINDOWS\\system32\\drivers\\VBoxMouse.sys";
		files[1] = "C:\\WINDOWS\\system32\\drivers\\VBoxGuest.sys";
		files[2] = "C:\\WINDOWS\\system32\\drivers\\VBoxSF.sys";
		files[3] = "C:\\WINDOWS\\system32\\drivers\\VBoxVideo.sys";
		for (size_t i = 0; i < 4; i++) {
			if (GetFileAttributesA(files[i].c_str()) != INVALID_FILE_ATTRIBUTES) {
				internal::debug("detected Virtual Box by system driver file " + files[i]);
				isVbox = true;
				break;
			}
		}

		// DETECT BASED ON OTHER SYSTEM FILES
		string files2[14];
		files2[0] = "C:\\WINDOWS\\system32\\vboxdisp.dll";
		files2[1] = "C:\\WINDOWS\\system32\\vboxhook.dll";
		files2[2] = "C:\\WINDOWS\\system32\\vboxmrxnp.dll";
		files2[3] = "C:\\WINDOWS\\system32\\vboxogl.dll";
		files2[4] = "C:\\WINDOWS\\system32\\vboxoglarrayspu.dll";
		files2[5] = "C:\\WINDOWS\\system32\\vboxoglcrutil.dll";
		files2[6] = "C:\\WINDOWS\\system32\\vboxoglerrorspu.dll";
		files2[7] = "C:\\WINDOWS\\system32\\vboxoglfeedbackspu.dll";
		files2[8] = "C:\\WINDOWS\\system32\\vboxoglpackspu.dll";
		files2[9] = "C:\\WINDOWS\\system32\\vboxoglpassthroughspu.dll";
		files2[10] = "C:\\WINDOWS\\system32\\vboxservice.exe";
		files2[11] = "C:\\WINDOWS\\system32\\vboxtray.exe";
		files2[12] = "C:\\WINDOWS\\system32\\VBoxControl.exe";
		files2[13] = "C:\\program files\\oracle\\virtualbox guest additions\\";
		for (size_t i = 0; i < 14; i++) {
			if (filesystem::fileExists(files2[i].c_str())) {
				internal::debug("detected virtual box by system file " + files2[i]);
				isVbox = true;
				break;
			}
		}

		// DETECT VIRTUALBOX MAC ADDRESS
		//if (hardware::macVendorMatches((char*) "\x08\x00\x27")) {
		//	internal::debug("Detected a likely virtualbox MAC address");
		//	isVbox = true;
		//}

		// DETECT VIRTUALBOX DEVICES
		HANDLE h;
		string devices[4];
		devices[0] = "\\\\.\\VBoxMiniRdrDN";
		devices[1] = "\\\\.\\pipe\\VBoxMiniRdDN";
		devices[2] = "\\\\.\\VBoxTrayIPC";
		devices[3] = "\\\\.\\pipe\\VBoxTrayIPC";
		for (size_t i = 0; i < 4; i++) {
			h = CreateFileA(devices[i].c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (h != INVALID_HANDLE_VALUE) {
				internal::debug("Detected virtualbox via device " + devices[i]);
				isVbox = true;
			}
		}
		
		// CHECK FOR VIRTUALBOX PROCESSES
		int res = false;
		HANDLE hpSnap;
		PROCESSENTRY32 pentry;

		hpSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hpSnap != INVALID_HANDLE_VALUE) {
			pentry.dwSize = sizeof(PROCESSENTRY32);
		}


		if (!Process32First(hpSnap, &pentry)) {
			CloseHandle(hpSnap);
		}

		do {
			if (lstrcmpi(pentry.szExeFile, L"vboxservice.exe") == 0) {
				internal::debug("VirtualBox traced using vboxservice.exe process");
				isVbox = true;
			}
			if (lstrcmpi(pentry.szExeFile, L"vboxtray.exe") == 0) {
				internal::debug("VirtualBox traced using vboxtray.exe process");
				isVbox = true;
			}
		} while (Process32Next(hpSnap, &pentry));
		


		if (isVbox) {
			exit(0);
		}

		internal::debug("VirtualBox check Passed");
	}

	void requireNotVmware() {

		bool isVmware = false;
		// TEST FOR COMMON VMWARE REG KEYS
		if (
			registry::keyValueExistsAndContainsStr(HKEY_LOCAL_MACHINE, "HARDWARE\\DEVICEMAP\\Scsi\\Scsi Port 0\\Scsi Bus 0\\Target Id 0\\Logical Unit Id 0", "Identifier", "VMWARE") ||
			registry::keyValueExistsAndContainsStr(HKEY_LOCAL_MACHINE, "HARDWARE\\DEVICEMAP\\Scsi\\Scsi Port 1\\Scsi Bus 0\\Target Id 0\\Logical Unit Id 0", "Identifier", "VMWARE") ||
			registry::keyValueExistsAndContainsStr(HKEY_LOCAL_MACHINE, "HARDWARE\\DEVICEMAP\\Scsi\\Scsi Port 2\\Scsi Bus 0\\Target Id 0\\Logical Unit Id 0", "Identifier", "VMWARE")
			) {
			internal::debug("Detected Vmware via Scsi-related registry key");
			isVmware = true;
		}

		// CHECK FOR VMWARE TOOLS REG KEY
		if (registry::keyExists(HKEY_LOCAL_MACHINE, "SOFTWARE\\VMware, Inc.\\VMware Tools")) {
			internal::debug("Detected Vmware via Vmware tools reg key");
			isVmware = true;
		}

		// CHECK FOR VMWARE SYS FILES
		if (
			filesystem::fileExists("C:\\WINDOWS\\system32\\drivers\\vmmouse.sys") ||
			filesystem::fileExists("C:\\WINDOWS\\system32\\drivers\\vmhgfs.sys")
			) {
			internal::debug("Detected Vmware via sys files");
			isVmware = true;
		}

		// CHECK FOR VMWARE DEVICES
		HANDLE h;
		string devices[2];
		devices[0] = "\\\\.\\HGFS";
		devices[1] = "\\\\.\\vmci";
		for (size_t i = 0; i < 2; i++) {
			h = CreateFileA(devices[i].c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (h != INVALID_HANDLE_VALUE) {
				internal::debug("Detected Vmware via related device");
			}
		}


		if (isVmware) exit(0);
		internal::debug("VmWare checks passed");
	}

	void requireNotBochsEmulator() {
		bool isBochs = false;

		// UNFORTUNATELY, CAN'T IMPLEMENT A LOT OF THE CPU CHECKS IN https://github.com/dsnezhkov/pufferfish/blob/master/pafish/bochs.c 
		// SINCE VISUAL STUDIO DOESN'T ALLOW INLINE ASSEMBLY

		// CHECK FOR COMMON BOCHS REG KEYS
		if (registry::keyValueExistsAndContainsStr(HKEY_LOCAL_MACHINE, "HARDWARE\\Description\\System", "SystemBiosVersion", "BOCHS")) {
			internal::debug("Detected Bochs emulator via reg key");
			isBochs = true;
		}

		if (isBochs) exit(0);
		internal::debug("Bochs emulator checks passed");
	}

	void requireNotQemu() {

		// CHECK FOR COMMON REG KEYS
		bool isQemu = false;

		if (registry::keyValueExistsAndContainsStr(HKEY_LOCAL_MACHINE, "HARDWARE\\DEVICEMAP\\Scsi\\Scsi Port 0\\Scsi Bus 0\\Target Id 0\\Logical Unit Id 0", "Identifier", "QEMU") ||
			registry::keyValueExistsAndContainsStr(HKEY_LOCAL_MACHINE, "HARDWARE\\Description\\System", "SystemBiosVersion", "QEMU")
			) {
			internal::debug("QEMU detected via reg key");
			isQemu = true;
		}


		if (isQemu) exit(0);
		internal::debug("QEMU checks passed");
	}

	void requireNoDebuggerAttached() {

		bool isDebuggerAttached = false;

		// USE WINDOWS API TO CHECK 
		if (IsDebuggerPresent()) {
			internal::debug("Determined that a debugger is present!");
			isDebuggerAttached = true;
		}

		// TRY DOING SOMETHING THAT SHOULDN'T DROP AN ERROR IF WE'RE DEBUGGING
		/* COMMENTING OUT BECAUSE CREATES FALSE POSITIVES
		DWORD err = 99;
		SetLastError(err);

		OutputDebugStringA( "useless");
		if (GetLastError() == err) {
			internal::debug("Determined that a debugger is present via error handling!");
			isDebuggerAttached = true;
		}
		*/

		if (isDebuggerAttached) exit(0);
		internal::debug("Passed debugger check!");
	}

	void requireComputerName(char* computerName) {

		char* outBuffer = new char[MAX_COMPUTERNAME_LENGTH + 1];
		DWORD bufferSize = MAX_COMPUTERNAME_LENGTH + 1;
		BOOL success = GetComputerNameA(outBuffer, &bufferSize);
		if (!success) {
			internal::debug("Failed to get computer name!");
			exit(0);
		}

		if (!strcmp(computerName, outBuffer) == 0) {
			internal::debug("Computer name mismatch!");
			exit(0);
		}

		internal::debug("Computer name check passed");
	}

	void requireFileExists(char* fileAbsolutePath) {
		
		if (!filesystem::fileExists(fileAbsolutePath)) {
			internal::debug("expected filed didnt't exist");
			exit(0);
		}
		internal::debug("File exists check passed");
	}

	void requireDirectoryExists(char* directoryAbsolutePath) {
		
		if (!filesystem::directoryExists(directoryAbsolutePath)) {
			internal::debug("Directory check passed");
			exit(0);
		}
		
	}

	void requireCommonMsOfficeApplicationsInstalled() {
		bool officeRegKeysPresent = false;
		bool installDirPresent = false;
		bool classPresent = false;
		bool appPathKeysPresent = false;

		if (
			registry::keyExists(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Office\\PowerPoint") ||
			registry::keyExists(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Office\\Word") ||
			registry::keyExists(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Office\\Excel") ||
			registry::keyExists(HKEY_LOCAL_MACHINE, "SOFTWARE\Microsoft\\Office\\Outlook")
			) {
			officeRegKeysPresent = true;
		}


		if (registry::keyExists(HKEY_CLASSES_ROOT, "Word.Application\CurVer")) {
			classPresent = true;
		}


		if (registry::keyExists(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\OUTLOOK.EXE") ||
			registry::keyExists(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\powerpnt.exe") ||
			registry::keyExists(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\excel.exe") ||
			registry::keyExists(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\Winword.exe")) {
			appPathKeysPresent = true;
		}



		if (officeRegKeysPresent || classPresent || appPathKeysPresent) {
			internal::debug("Office common apps check passed");

		}
		else {
			internal::debug("missing common office apps");
			exit(0);
		}
	}

	void requireMouseActivity(int duration) {
		POINT position1, position2;
		GetCursorPos(&position1);
		Sleep(duration * 1000); /* Sleep time */
		GetCursorPos(&position2);
		if ((position1.x == position2.x) && (position1.y == position2.y)) {
			/* No mouse activity during the sleep */
			internal::debug("Mouse check failed");
			exit(0);
		}
		else {
			/* Mouse activity during the sleep */
			internal::debug("Mouse check passed");
		}
	}

	void requireUsername(char* username) {

		char* usernameCopy = new char[strlen(username) + 1];
		strcpy(usernameCopy, username);
		size_t UNAME_LEN_MAX = 256;
		LPSTR currentUsername = new char[UNAME_LEN_MAX + 1];
		DWORD currentUsernameLength = UNAME_LEN_MAX + 1;
		BOOL success = GetUserNameA(currentUsername, &currentUsernameLength);

		// LOWERCASE BOTH CHAR STRINGS
		LPSTR currentUsernameLower = CharLowerA(currentUsername);
		LPSTR usernameLower = CharLowerA(usernameCopy);
		if (!success || strcmp(currentUsernameLower, usernameLower) != 0) {
			internal::debug("Username check failed!");
			delete[] usernameCopy;
			exit(0);
		}
		internal::debug("Username check passed");
		delete[] usernameCopy;
	}

	void requireStdUsernameCheck() {

		size_t UNAME_LEN_MAX = 256;
		LPSTR currentUsername = new char[UNAME_LEN_MAX + 1];
		DWORD currentUsernameLength = UNAME_LEN_MAX + 1;
		string forbiddenUsernames[3];
		forbiddenUsernames[0] = "malware";
		forbiddenUsernames[1] = "sandbox";
		forbiddenUsernames[2] = "virus";
		BOOL success = GetUserNameA(currentUsername, &currentUsernameLength);
		if (!success) {
			internal::debug("Username check failed, failed to get username");
			exit(0);
		}

		LPSTR currentUsernameLower = CharLowerA(currentUsername);
		for (size_t i = 0; i < 3; i++) {
			
			
			if (strcmp(currentUsernameLower, forbiddenUsernames[i].c_str()) == 0) {
				internal::debug("std username check failed");
				exit(0);
			}
		}
		internal::debug("Std username check passed");
	}

	void requireStdPathCheck() {

		size_t MAX_PATH_VAR = 2047;
		string varName = "PATH";
		DWORD pathBufferLength = MAX_PATH_VAR + 1;
		char* pathBuffer = new char[MAX_PATH_VAR + 1];
		DWORD currentModulePathLength = MAX_PATH + 1;
		char* currentModulePath = new char[MAX_PATH + 1];

		DWORD pathStrLength = GetEnvironmentVariableA(varName.c_str(), pathBuffer, pathBufferLength);
		currentModulePathLength = GetModuleFileNameA(NULL, currentModulePath, currentModulePathLength);

		string badWords[4];
		badWords[0] = "malware";
		badWords[1] = "virus";
		badWords[2] = "sandbox";
		badWords[3] = "sample";

		
		char* pathBufferLower = CharLowerA(pathBuffer);
		char* currentModulePathLower = CharLowerA(currentModulePath);

		string path = pathBufferLower;
		string modulePath = currentModulePathLower;
		for (size_t i = 0; i < 4; i++) {
			if (
				path.find(badWords[i]) != string::npos || 
				modulePath.find(badWords[i]) != string::npos
				) {
				internal::debug("Std path check failed");
				exit(0);
			}

		}

		internal::debug("Std path check passed");
		// END: CLEANUP
		delete[] pathBuffer;
		delete[] currentModulePath;
	}

	void requireStdDriveSizeCheck(int minDiskSizeGb, int minFreeDiskSizeGb) {

		// CREATE 64-BIT INTEGERS TO RECEIVE NUMBERS FROM THE API CALL
		ULARGE_INTEGER freeBytesAvailableToCaller, totalNumberOfBytes, totalNumberOfFreeBytes; 

		BOOL success = GetDiskFreeSpaceExA("C:\\", &freeBytesAvailableToCaller, &totalNumberOfBytes, &totalNumberOfFreeBytes);
		if (!success) {
			internal::debug("Failed to get disk size");
			exit(0);
		}
		
		// CONVERT BYTES TO GB VIA BIT SHIFT
		// DIVIDING BY 1024 IS A LSR OF 10 (>> 10), AND CONVERTING FROM BYTES TO GB REQUIRES THIS 3 TIMES, HENCE AN LSR OF 30
		ULONGLONG freeGigabytes, totalGigabytes;
		freeGigabytes = totalNumberOfFreeBytes.QuadPart >> 30; // DIVIDE BY 1024 IS A LSR 10 ( >> 10), DIVIDE BY 1024 3X TO GO FROM BYTES TO KB, KB TO MB, MB TO GB
		totalGigabytes = totalNumberOfBytes.QuadPart >> 30;
		
		// NOW THAT THE 64-BIT INTEGERS ARE SUFFICIENTLY SMALL, COPY THEM INTO UNSIGNED INTS SO WE CAN COMPARE THEM
		unsigned int totalDiskGb = totalGigabytes;
		unsigned int freeDiskGb = freeGigabytes;

		if (minDiskSizeGb > totalDiskGb) {
			internal::debug("Minimum disk size check failed");
			exit(0);
		}
		if (minFreeDiskSizeGb > freeDiskGb) {
			internal::debug("Minimum free disk space check failed!");
			exit(0);
		}
		internal::debug("Disk size check passed");
	}

	void requireSleepIsNotPatched() {
		DWORD start = GetTickCount();
		Sleep(500);
		if ((GetTickCount() - start) < 450) {
			internal::debug("Sleep is patched!");
			exit(0);
		}

		internal::debug("Sleep patch check passed");
	}

	void requireMinimumNumProcessors(int numProcessors) {
		SYSTEM_INFO sysInfo;
		GetSystemInfo(&sysInfo);

		if (sysInfo.dwNumberOfProcessors < numProcessors) {
			internal::debug("Processor count check failed");
			exit(0);
		}

		internal::debug("Processor count check passed");
	}

	void requireMinimumGbMemory(int n) {
		
		// WEIRD EXPRESSION TO PREVENT SUB-EXPRESSION OVERVLOW
		ULONGLONG expectedSystemMemoryInKb = n;
		expectedSystemMemoryInKb = expectedSystemMemoryInKb * 1024 * 1024;
		ULONGLONG systemMemoryInKb;
		GetPhysicallyInstalledSystemMemory(&systemMemoryInKb);

		if (expectedSystemMemoryInKb > systemMemoryInKb) {
			internal::debug("Failed system memory check");
			exit(0);
		}

		internal::debug("System Memory check passed");



	}

	void requireSystemUptime(int minutes) {
		DWORD expectedUptimeInMs = minutes * 60 * 1000; // MINUTES * 60 = SECONDS * 1000 = MILLISECONDS
		DWORD actualUptimeInMs = GetTickCount();

		if (actualUptimeInMs < expectedUptimeInMs) {
			internal::debug("Expected system uptime check failed");
			exit(0);
		}

		internal::debug("System uptime check passed");

	}

	void requirePrinterInstalled(string printerSubstring) {

		DWORD printerInfoByteSize = sizeof(PRINTER_INFO_1) * 40;	// SPACE FOR 40 PRINTER_INFO_1 structures
		LPBYTE printerInfo = new BYTE[printerInfoByteSize];
		

		DWORD bytesOut, printerCount;

		BOOL success = EnumPrintersA(PRINTER_ENUM_CATEGORY_ALL, NULL, 1, printerInfo, printerInfoByteSize, &bytesOut, &printerCount);
	}

}