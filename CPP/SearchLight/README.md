# SearchLight
Detect Hooked System calls and determine which system calls to avoid. Also detects hooked Win32 API calls (only interesting ones used for process injection)

## Syscall Hook Detection
Syscall hooking typically works by inserting a `JMP` instruction at the beginning of the call such that execution jumps into the AV/EDR's code which can examine arguments and behavior. 
The AV/EDR code will then do it's thing and then jump back to the normal syscall (if it decides it's okay).

Normally, the syscalls would start with a `MOV` instruction, e.g. `MOV r10,rcx` which translaces to `4c 8b d1 b8`. We can iterate over the exported functions in NTDLL and look to see if the first 
instruction of each function looks like this. If so, then the function has probably not been hooked. If the function looks like something else, it is (probably) being hooked. Note that this will 
produce some false positives (per ired.team):

```
NtGetTickCount
NtQuerySystemTime
NtdllDefWindowProc_A
NtdllDefWindowProc_W
NtdllDialogWndProc_A
NtdllDialogWndProc_W
ZwQuerySystemTime
```

These will be ignored by the program.

(By looking for a `JMP` instruction, we can find which syscalls have almost certainly been hooked. `e9` is the opcode for a `JMP`, if a function starts with it then we can conclude with a high degree 
of certainty that the function has been hooked.)

Reference: https://www.ired.team/offensive-security/defense-evasion/detecting-hooked-syscall-functions

## API Hook Detection

Reference: https://www.ired.team/offensive-security/defense-evasion/bypassing-cylance-and-other-avs-edrs-by-unhooking-windows-apis

## Usage

```
SearchLight.exe
```