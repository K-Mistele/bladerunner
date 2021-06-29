# Lightwave
Lightwave is a C++ shellcode loader that uses `CreateRemoteThread` to inject shellcode into a process. 

## How it Works
1. Create a new process by launching a chosen executable
2. Use the handle on the launched process to reserve memory in the new process
3. Inject shellcode into that new RWX memory
4. Create a new thread in that process to run the shellcode

## Usage
```
lightwave.exe executablePath  [toggleDebug]
```

Note: all arguments are positional.
* `executablePath`: path to the executable to launch. Make sure to double quote if there are spaces or special characters
* `toggleDebug`: if specified as "debug", will run debug mode which will give you verbose debugging information