#include "procenum.h"
#include <vector>
#include <string>
#include <Windows.h>
#include <TlHelp32.h>
#include <iostream>

using std::string;
using std::vector;
using std::cout;
using std::endl;

// ENUMERATE INJECTABLE PROCESSES
vector<KmProcessInfo> KmEnumInjectableProcesses() {

	HANDLE snapshot = NULL;
	HANDLE openProcess = NULL;
	HANDLE openThread = NULL;
	PROCESSENTRY32 processEntry = { sizeof(PROCESSENTRY32) };
	THREADENTRY32 threadEntry = { sizeof(THREADENTRY32) };
	BOOL validProcess = false;
	vector<KmProcessInfo> processInfoList = vector<KmProcessInfo>();

	// GET THE FIRST PROCESS
	cout << "[+] Attempting to get first process" << endl;
	snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS | TH32CS_SNAPTHREAD, 0);
	validProcess = Process32First(snapshot, &processEntry);
	if (!validProcess) {
		cout << "[-] Failed to get process" << endl;
	}


	// ITERATE ACROSS PROCESSES, ATTEMPTING 
	while (validProcess) {
		openProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, processEntry.th32ProcessID);

		// IF OPENING THE PROCESS WAS SUCCESSFUL
		if (openProcess) {
			struct KmProcessInfo procInfo;

			// GET THE PROCESS ID
			procInfo.processId = processEntry.th32ProcessID;

			// GET THE PROCESS NAME, PERFORMING WIDESTRING TO MULTIBYTE STRING CONVERSION
			char defaultChar = '?';
			char* exeName = new char[260];
			WideCharToMultiByte(CP_ACP, 0, processEntry.szExeFile, 260, exeName, 260, &defaultChar, NULL);
			procInfo.processName = string(exeName);

			// GET THE NUMEBR OF THREADS
			int numThreads = 0;
			BOOL validThread = Thread32First(snapshot, &threadEntry);
			while (validThread) {
				if (threadEntry.th32OwnerProcessID == processEntry.th32ProcessID) {
					numThreads++;
				}
				validThread = Thread32Next(snapshot, &threadEntry);

			}
			procInfo.numThreads = numThreads;


			processInfoList.push_back(procInfo);
			CloseHandle(openProcess);
			
		}

		// TRY TO OPEN THE NEXT PROCESS
		validProcess = Process32Next(snapshot, &processEntry);
	};
	return processInfoList;
}