#include "sandbox_detection.h"
#include "Windows.h"
#include <iostream>
#include <string>
#include <cstring>
#include <Windows.h>
#include <WinDNS.h>
#include "utils.h"
#include <TlHelp32.h>



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
		internal::debug("Grabbed domain name!");
		internal::debug(currentDomainStr);
		

		// CHECK TO SEE IF THEY MATCHED
		if (strcmp(domain, currentDomainStr) != 0) {
			internal::debug("Domain does not match, exiting");
			exit(0);
		}
		internal::debug("domains match!");

		// FREE MEMORY 
		delete[] currentDomainStr;
		delete[] currentDomainWideStr;
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
	}
}