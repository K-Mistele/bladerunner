# Lightwave
Lightwave is a C++ shellcode loader that uses `NtQueueApcThread` to inject shellcode into a process.

**As of testing on 07/09/2021, this loader will bypass both CrowdStrike and Windows Defender ATP**

## How it Works
1. Open the target process
2. Allocate memory in read-write mode
3. write the shellcode
4. re-protect the memory to read-execute 
5. Iterate across threads, Queuing an APC for each.

## Caveats
Note that this technique not guarantee execution: there must be at least one thread in the target process in an alertable state. This only occurs when a thread has called one of the following functions
with the correct flags:
* `SleepEx`
* `WaitForSingleObjectEx`
* `WaitForMultipleObjectsEx`
* `SignalObjectAndWait`
* `MsgWaitForMultipleObjectsEx`

Clearly, I/O bound processes are ideal. Additionally, the more threads that a process has, the more likely you are to find one in the right state. 
However, if multiple threads are in the alertable state, you may get multiple beacons/shells. 
I have had a great deal of luck with Firefox (the main process can sometimes have over 70 threads, and is heavily I/O bound - I got about 8 beacons back); other browsers and network-heavy applications 
are ideal as well.

Additionally, it is important to note that if the thread you're in dies, your beacon/shell will as well, so you should try to migrate ASAP (I have not experienced this while testing with injecting into `firefox.exe`, but I
am told that it can happen)


## Customization
Make sure to edit the source with the following: 
* your shellcode, encrypted and base64 encoded
* set your key - either bytewise xor or a key string
* change the `DECRYPTION_MODE` enum in the configuration to use the right decryption method
* per the addendum below, you may need to modify the memory protection options in `VirtualAllocEx` and `VirtualProtectEx`

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
lightwave.exe targetpid [debug]
```