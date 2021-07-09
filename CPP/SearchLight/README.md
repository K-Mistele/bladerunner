# SearchLight
Provides headers to perform various enumerations. Included:

## Process Enumeration
Search for injectable processes (i.e. processes that code can be opened and written to) and enumerates the number of threads. This is useful for APC Queue Injection techniques - the more threads a 
target process has, the more you are likely to get one that's in the right state. Does not perform injections, simply tries to get a `PROCESS_ALL_ACCESS` handle and enumerates threads.

Requires no parameters or arguments.
