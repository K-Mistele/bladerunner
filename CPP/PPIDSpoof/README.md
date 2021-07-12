# PPIDSpoof
A proof-of-concept that launches an executable and changes the PPID of the process. May be useful for evasion. Does not handle passing command line arguments to the target.

## Usage
Launches an executable specified by the second argument, and sets its parent process ID to the value specified by the first argument. The Parent PID must be the PID of a valid process that you have `PROCESS_ALL_ACCESS` to (processes running as your user or at a lower integrity level). Make sure to quote executable paths if there are spaces or special characters.
```
PPIDSpoof.exe [Parent PID to spoof] [Executable to Launch]
```

Example:
```
PPIDSpoof.exe 9720 notepad.exe
```