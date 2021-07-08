# BlackLight
GhostLight is a C++ shellcode loader that uses kernel system calls to inject shellcode into a process. 
Specifically, it packs a `.asm` file that contains the system calls it uses pulled from `ntdll.dll` to bypass userland API hooking. 
Note that this will not bypass hooking in the kernel. The assembly file contains implementations of the syscalls for Windows 7, Windows 8, and Windows 10, but for x64 Windows only. x32 is not supported.

Syscall assembly file is generated with https://github.com/jthuraisamy/SysWhispers. I picked out only the ones I need to minimize the footprint:

## Configuring Visual Studio 
To successfully compile this project, you need to ensure that masm is enabled for the project. Importing the solution should do this for you, but if not:

1.In Visual Studio, go to Project ? Build Customizations... and enable MASM. Depending on your Visual studio version, you may need to right-click on your project -> Build Dependencies -> Build Customizations... 
2.In the Solution Explorer, add the .h and .asm files to the project as header and source files, respectively.
3.Go to the properties of the ASM file, and set the Item Type to Microsoft Macro Assembler.
4.Ensure that the project platform is set to x64. The `.asm` file does not support x32 windows, so neither does this project.

### Very important addendum
Lots of shellcode loaders take the easy way out by using `VirtualAllocEx` to allocate `PAGE_EXECUTE_READWRITE` (rwx) memory. This type of memory is often considered suspicious, 
and it is obvious to AV/EDR and may be easily detected.
In this loader, I have elected to allocate the memory as `PAGE_READWRITE`, and then use `VirtualProtectEx` to change the protection to `PAGE_EXECUTE` after injecting the shellcode but
BEFORE creating the thread. 

**HOWEVER** - if you are using a self-decoding or self-decrypting payload, this will not work, and you will get an error and a crash. I have designed the loader to use non-self-editing 
shellcode that is encrypted in some way, and then encoded in base64: the loader decodes the shellcode and then decrypts it before executing it. Instead of relying on self-editing shellcode
consider using normal shellcode that is not self-modifying, and then encrypting it yourself (easy to do with Python and XOR).

**If you supply your own encryption on a non-self-encrypting shellcode, you must modify the** `decrypt` **function to perform the decryption.**

**If you are determined to use self-decrypting or self-decoding shellcode, you probably need to nuke the decryption routing in the loader, and in the call to** `VirtualProtectEx` 
**you need to specify** `PAGE_EXECUTE_READWRITE` **instead of** `PAGE_EXECUTE`. **Be forewarned that this may increase detections.**

## Usage
```
ghostlight.exe LaunchType Target [Debug]
```

Note: all arguments are positional.
* `LaunchType`: required positional argument. Permitted values:
	* `PID`: target an existing process
	* `EXEC`: specify an executable to run via command line
* `Target`: required positional argument. Permitted values:
	* if `LaunchType` is `PID`: the PID of an existing process to inject shellcode into
	* if `LaunchType` is `EXEC`: the path (and arguments?) of an executable to launch. Make sure to double-quote this value if there are spaces.
* `Debug`: optional positional argument. If set to `debug`, then will print extra debugging information.

### Examples:
```
ghostlight.exe PID 5268 debug

ghostlight.exe EXEC "C:/program files/internet explorer/iexplore.exe" debug

ghostlight.exe EXEC "powershell.exe -windowstyle hidden" debug
```