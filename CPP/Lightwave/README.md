# Lightwave
Lightwave is a C++ shellcode loader that uses `CreateRemoteThread` to inject shellcode into a process. 

## How it Works
1. Create a new process by launching a chosen executable
2. Use the handle on the launched process to reserve memory in the new process
3. Inject shellcode into that new RWX memory
4. Create a new thread in that process to run the shellcode

## Customization
Make sure to edit the source with the following: 
* your shellcode
* change the right decryption routine, and ensure that it's called in main
* per the addendum below, you may need to modify the memory protection options in `VirtualAllocEx` and `VirtualProtectEx`

### Very important addendum
Lots of shellcode loaders take the easy way out by using `VirtualAllocEx` to allocate `PAGE_EXECUTE_READWRITE` (rwx) memory. This type of memory is often considered suspicious, 
and it is obvious to AV/EDR and may be easily detected.
In this loader, I have elected to allocate the memory as `PAGE_READWRITE`, and then use `VirtualProtectEx` to change the protection to `PAGE_EXECUTE` after injecting the shellcode but
BEFORE creating the thread. 

**HOWEVER** - if you are using a self-decoding or self-decrypting payload, this will not work, and you will get an error and a crash. I have designed the loader to use non-self-editing 
shellcode that is encrypted in some way, and then encoded in base64: the loader decodes the shellcode and then decrypts it before executing it. Instead of relying on self-editing shellcode
consider using normal shellcode that is not self-modifying, and then encrypting it yourself (easy to do with Python and XOR).

**If you supply your own encryption on a non-self-encrypting shellcode**, you must modify the `decrypt` function to perform the decryption.**

**If you are determined to use self-decrypting or self-decoding shellcode, you probably need to nuke the decryption routing in the loader, and in the call to** `VirtualProtectEx` 
** you need to specify `PAGE_EXECUTE_READWRITE` instead of `PAGE_EXECUTE`. Be forewarned that this may increase detections.**

## Usage
```
lightwave.exe LaunchType Target [Debug]
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
Lightwave.exe PID 5268 debug

Lightwave.exe EXEC "C:/program files/internet explorer/iexplore.exe" debug

Lightwave.exe EXEC "powershell.exe -windowstyle hidden" debug
```