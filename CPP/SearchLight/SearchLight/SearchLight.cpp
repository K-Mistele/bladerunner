#include <iostream>
#include <Windows.h>
#include <string>
#include <vector>
#include "procenum.h"

using std::string;
using std::to_string;
using std::vector;
using std::cout;
using std::endl;

void debug(string msg) {
	cout << "[+] " << msg << endl;
}

int main()
{
	vector<KmProcessInfo> injectableProcesses = KmEnumInjectableProcesses();
	string lenStr = to_string(injectableProcesses.size());
	debug("Found " + lenStr + " injectable processes");

	for (size_t i = 0; i < injectableProcesses.size(); i++) {
		struct KmProcessInfo pInfo = injectableProcesses.at(i);
		debug("Injectable Process: " + pInfo.processName + " (PID " + to_string(pInfo.processId) + "); number of threads: " + to_string(pInfo.numThreads));
	}


	return 0;
}