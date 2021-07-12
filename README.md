# BladeRunner
A collection of shellcode runners and associated tools for AV/EDR bypass. 

## Project Structure 
* `PyTools` - a collection of Python tools to facilitate using the various runners
* `CPP` - C++ shellcode runners and an accompanying tool or two

## OpSec
Please observe the following for opsec purposes:
* Do not commit your shellcode with the runner. if you have added shellcode and then find you need to make a commit, either temporarily remote it before staging the changes, or use the "stage hunk" feature
* Do not commit your encryption key(s). 
* Make sure that if you use `DeepStoneCrypt` to encrypt your payloads (always encrypt your payloads _somehow_), that you do not commit either the encrypted or unencrypted files. The project's `.gitignore` should ignore `.txt` and `.bin` files, but if you save it with a different file extension, git may pick it up. Please be careful in this regard.


# A Brief Note on EDR Software
Most EDRs [citation needed] use userland API hooking to detect malicious behaviours by identifying sequences of Windows API calls that are considered suspicious. The classic example of this would be `OpenProcess`, `VirtualAllocEx`, `WriteProcessMemory`, and `CreateRemoteThread`. 

Another common form that this takes is inspecting arguments to certain calls. For example, calling `MiniDumpWriteDump` on `notepad.exe` may be allowed, whereas calling it on `lsass.exe` will result in a detection and the EDR will most likely prevent execution. 

## Custom Syscalls
It is sometimes possible to bypass these behavioral detections using "direct syscalls" or "custom syscalls". To understand this, it's necessary to understand what the Windows API does. For example, when you call attempt to open a process, the `OpenProcess` API call will perform various sanity checks (e.g. for `NULL` pointers), type conversions, etc.. Then, it will call an unexported function in `NTDLL.dll`, in this case `NtOpenProcess`. This call will set up the registers for the system call, including moving the syscall number into `%rcx`, before executing the `syscall` instruction. 

**Some** EDR's will hook the Windows API function. In this case, it is possible to call `GetProcAddress` to locate the appropriate unexported function (`NtOpenProcess`) and call this directly. 

**However**, other EDR's like CrowdStrike will actually hook into the unexported `NTDLL` functions, in which case it becomes necessary to take a different approach. One approach is to locate the function and "unhook" it by overwriting the `jmp` instruction(s) with the correct instructions for the function. 

Another approach that I opted to use for these runners is to in effect provide your own system calls: to copy the instructions out of an unhooked `NTDLL.dll` file, to build them into your executable, and to execute these directly when necessary. 

### SysWhispers

Enter [SysWhispers](https://github.com/jthuraisamy/SysWhispers). Syswhispers builds an assembly file containing any of the system calls that you need (e.g. `NtOpenProcess`, `NtAllocateVirtualMemory`, etc), and will allow you to specify multiple Operating System versions to support. It handles building in compatibility for different versions and service packs, and makes your life a lot easier. Run it on your dev machine and selecte the functions you need and the OS versions you want to support, and it will provide a `.asm` (assembly) file and a header file for you to include in your project. This technique will bypass userland hooking completely. 

Note that [SysWhispers2](https://github.com/jthuraisamy/SysWhispers2) was released, which will automatically generate cross-version calls for you. However, I had not found this at the time of writing this project.

## Signature Detection
Usually, EDRs will also use some form of signature detection as well. It is therefore important to change your binary's hash somehow before compiling it. Adding in strings as well as changing shellcode and keys that are coded into the binary will do this as well. 

## BUT, There is No Silver Bullet
EDRs are constantly improving and evolving. Even using SysWhispers and encrypted shellcode, my loader was consistently getting caught by CrowdStrike because I was using the `CreateRemoteThread` technique, which is easy to detect via other forms of telemetry other than API Hooking (process hacker and process explorer for example will allow you to see process' open handles, threads, allocated memory, etc.).

I was able to bypass Crowdstrike by using a different injection technique that is harder to detect via telemetry (`NtQueueApcThread`), but it is entirely possible that this technique might be caught in the future. It is therefore often necessary to experiment not only with different techniques for bypassing userland API hooking, but also with different techniques for process injection and shellcode execution.

On that note, **these techniques and loaders worked as of 7/10/2021**. Testing was done in a sandbox environment in an opsec-safe manner with Windows Defender ATP and Crowdstrike enabled, but with automatic sample submission _off_. No samples were submitted to VirusTotal etc., so at the time of this writing, these loaders should not have been burned.

**However**, there is no guarantee that these loaders or the techniques that they use will not be detected in the future - it may be a year from now, or it may be tomorrow. Mileage may vary against other EDRs as well.

# Runners
Each Runner has its own `README.md` that goes into much more detail than this section - this section simply provides an overview of each runner, it's technique, and it's applications. 

### Blacklight
**Successfully bypasses Defender ATP & Crowdstrike as of 7/10/2021**
The Blacklight runner (`CPP/Blacklight`) is _not_ a process injection loader. It allocates space in its own memory space for shellcode, and queues an APC on the main thread. It then uses `NtTestAlert` to force the thread to pick up the APC and execute it. 

* **Technique**: `NtQueueApcThread` on local process, `NtTestAlert` to force execution of shellcode
* **Evasion**: SysWhispers direct system calls, shellcode encryption, avoids `RWX` memory
* **Caveats**: 
  * does not natively support self-decoding and self-decrypting shellcode, although this can be enabled by changing the memory protection option in `NtProtectVirtualMemory`
  * does not perform process injection, and as a result will leave a standalone process running descended from `explorer.exe` or `cmd.exe`. This can be avoided using `PPIDSpoof`.

More details in the project-specific README (`CPP/Blacklight/README.md`).

### Lightwave
**Successfully bypasses Defender ATP & Crowdstrike as of 7/10/2021**
The Lightwave runner (`CPP/Lightwave`) uses a similar technique to Blacklight, but performs process injection. 
* **Technique**: `NtQueueApcThread` on a remote process, sprayed across all threads.
* **Evasion**: process injection, SysWhispers direct system calls, shellcode encryption, avoids RWX memory
* **Caveats**
  * While Blacklight uses `NtTestAlert` to force APC execution in a local process, it is not possible to force APC execution in a remote thread. Therefore, it is necessary to spray APCs across all threads in the remote process. Only threads in an `alertable` state can pick the APC up. 
  * The more threads that your target process has, the more likely it is that a thread will be in an `alertable` state to pick the APC up. Heavily I/O bound processes (e.g. browsers) are also great targes since they have a high number of threads and tend to have many threads in an `alertable` state. 

More details in the project-specific README (`CPP/Lightwave/README.md`)

### GhostLight

The GhostLight runner (`CPP/GhostLight`) uses the `CreateRemoteThread` technique to inject and execute shellcode into another target. This can be a good loader to use for purple teams since the technique is exceedingly common (many EDRs consider this technique synonymous with Cobalt Strike)
* **Technique**: `CreateRemoteThread` on a remote process
* **Evasion**: process injection, SysWhispers direct system calls, shellcode encryption, avoids RWX memory
* **Caveats**: 
  * While custom syscalls are used to bypass userland API hooking, this technique may be caught through other means of telemetry and monitoring with a sufficiently sophisticated EDR.
  * Defender and Crowdstrike both catch this loader.

More details in the project-specific README (`CPP/GhostLight/README.md`)

## Other Tools
 
## PPIDSpoof
PPID spoof can be used to spoof the PPID of a process in userland. It will fool things like task manager, procmon, process hacker, etc. but may not fool kernel-mode hooking.

More details in the project-specific README (`CPP/PPIDSpoof/README.md`)

## SearchLight
**As of 7/10/2021, this tool is not detected by Defender or Crowdstrike. Note that this tool is malicious only in intent - none of its behavior is explicitly malicious.**

SearchLight is designed to enumerate injectable processes. It iterates across processes, attempts to open them with `PROCESS_ALL_ACCESS`, and then iterates across the threads in each process counting them. SearchLight is idempotent, i.e. it does not modify process or threads - it only enumerates them.

It will produce a list of injectable processes with thread counts. This tool should make using Lightwave (see above) easier. 

More details in the project-specific README: (`CPP/SearchLight/Readme.md`)

## DeepStoneCrypt
This is a Python tool to perform encryption and encoding on binary payloads. Designed to provide encoding and encryption for the above Shellcode runners. 

Note that the runners currently only support Base64 encoding, but they all support either of the below encryption methods.

**Allows you to pick 0-1 encoding options:**
* Base32
* Base64

**Allows you to pick 0-1 encryption options:**
* Bytewise XOR encryption
* Key XOR encryption - repeatedly XOR a key across a payload. See implementation for details.

More details in the project-specific README (`PyTools/DeepStoneCrypt/README.md`)