# Lightwave
Lightwave is a C++ shellcode loader that uses `CreateRemoteThread` to inject shellcode into a process. 

## How it Works
1. Create a new process by launching a chosen executable
2. Use the handle on the launched process to reserve memory in the new process
3. Inject shellcode into that new RWX memory
4. Create a new thread in that process to run the shellcode

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
```