# BlackLight


**As of the time of testing at 07/08/2021, this loader successfully bypassed both CrowdStrike Falcon Sensor and Defender ATP**.

GhostLight is a C++ shellcode loader that uses direct/custom system calls to bypass userland API hooking and execute shellcode.

Specifically, it packs a `.asm` file that contains the system calls it uses pulled from `ntdll.dll` to bypass userland API hooking. 
Note that this will not bypass hooking in the kernel (e.g. via Process Instrumentation Callback). The assembly file contains implementations of the syscalls for Windows 7, Windows 8, and Windows 10, but for x64 Windows only. x32 is not supported.

Syscall assembly file is generated with https://github.com/jthuraisamy/SysWhispers. I picked out only the ones I need to minimize the footprint:
* `NtWriteVirtualMemory`
* `NtProtectVirtualMemory`
* `NtAllocateVirtualMemory`
* `NtQueueApcThread`
* `NtTestAlert`

## How it works
This loader does _not_ use process injection, it is a standalone loader and will result in a new running process.
1. Decode shellcode from Base64
2. Decrypt shellcode
3. Allocate memory in the current process in `PAGE_READWRITE` mode
4. Write the shellcode to memory
5. Change the memory protections on the shellcode to `PAGE_EXECUTE_READ`
6. Queue an APC for the main thread to execute the shellcode
7. Call `NtTestAlert` to force the process to flush the APC Queue and execute the shellcode

## Customization
Make sure to edit the source with the following: 
* your shellcode, encrypted and base64 decoded
* set your key - either bytewise xor or a key string
* change the `DECRYPTION_MODE` enum in the configuration to use the right decryption method
* per the addendum below, you may need to modify the memory protection options in `VirtualAllocEx` and `VirtualProtectEx`


## Very important addendum
Lots of shellcode loaders take the easy way out by using `VirtualAlloc` to allocate `PAGE_EXECUTE_READWRITE` (rwx) memory. This type of memory is often considered suspicious, 
and it is obvious to AV/EDR and may be easily detected.
In this loader, I have elected to allocate the memory as `PAGE_READWRITE`, and then use `VirtualProtect` to change the protection to `PAGE_EXECUTE_READ` after injecting the shellcode but
BEFORE creating the thread. 

**HOWEVER** - if you are using a self-decoding or self-decrypting payload, this will not work, and you will get an error (maybe) and a crash (definitely). I have designed the loader to use non-self-editing 
shellcode that is encrypted in some way, and then encoded in base64: the loader decodes the shellcode and then decrypts it before executing it. Instead of relying on self-editing shellcode
consider using normal shellcode that is not self-modifying, and then encrypting it yourself (easy to do with Python and XOR).

**If you supply your own encryption on a non-self-encrypting shellcode, you must modify the correct decryption function to perform the decryption with the apropriate key.**

**If you are determined to use self-decrypting or self-decoding shellcode, you probably need to nuke the decryption routine in the loader (unless you're performing your own encryption on top of it)
, and in the call to** `NtProtectVirtualMemory` **you need to specify** `PAGE_EXECUTE_READWRITE` **instead of** `PAGE_EXECUTE`. **Be forewarned that this may increase detections.**


## Configuring Visual Studio 
To successfully compile this project, you need to ensure that masm is enabled for the project. Importing the solution should do this for you, but if not:

1.In Visual Studio, go to Project ? Build Customizations... and enable MASM. Depending on your Visual studio version, you may need to right-click on your project -> Build Dependencies -> Build Customizations... 
2.In the Solution Explorer, add the .h and .asm files to the project as header and source files, respectively.
3.Go to the properties of the ASM file, and set the Item Type to Microsoft Macro Assembler.
4.Ensure that the project platform is set to x64. The `.asm` file does not support x32 windows, so neither does this project.


## Usage
```
BlackLight.exe

Blacklight.exe debug
```
