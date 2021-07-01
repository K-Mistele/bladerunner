# GreenLight
GreenLight is a C++ shellcode loader that creates an internal thread to run shellcode after decoding and decrypting it

## How it Works
1. Decode shellcode in memory
2. Decrypt shellcode in memory
3. Allocate memory for shellcode as read-write
4. Copy shellcode
5. Change protection to execute-only
6. Create thread


## Very important addendum
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
Greenlight.exe

Greenligh.exe debug
```