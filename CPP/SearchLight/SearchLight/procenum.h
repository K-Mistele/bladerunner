#include <vector>
#include <string>
#include <Windows.h>

using std::vector;
using std::string;
using std::to_string;

#pragma once

// STRUCT TO CONTAIN PROCESS INFORMATION
struct KmProcessInfo {
	string processName;
	DWORD processId;
	int numThreads;
};

// FUNCTION TO ITERATE ACROSS PROCESSES AND RETURN A LIST OF INJECTABLE PROCESSES
vector<KmProcessInfo> KmEnumInjectableProcesses();