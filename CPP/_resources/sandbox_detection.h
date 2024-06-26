#pragma once

#include <string>

namespace sandboxDetection {


	/// <summary>
	/// Anonymous namespace to contain private information, only available from within sandboxDetection namespace
	/// </summary>
	namespace internal {
		

		/// <summary>
		/// Print a debugging message if and only if in debug mode
		/// </summary>
		/// <param name="msg"></param>
		void debug(char* msg);

		/// <summary>
		/// Print a debugging message if and only if in debug mode
		/// </summary>
		/// <param name="msg"></param>
		void debug(std::string msg);

	}

	/// <summary>
	/// Enable debug mode. When enabled, check statuses will print
	/// </summary>
	void enableDebugMode();

	/// <summary>
	/// Require that the computer be joined to a given Active Directory Domain. Calls exit(0) if check fails.
	/// </summary>
	/// <param name="domain">The Active Directory Domain</param>
	/// <returns></returns>
	void  requireJoinedToDomain(char* domain);

	/// <summary>
	/// Require that the computer has the specified name. Calls exit(0) if check fails.
	/// </summary>
	/// <param name="computerName">The computer name to match against</param>
	void requireComputerName(char* computerName);

	/// <summary>
	/// Require that the current user has the specified username. Calls exit(0) if check fails.
	/// </summary>
	/// <param name="userName">The computer name to match against</param>
	void requireUsername(char* username);

	/// <summary>
	/// NOT IMPLEMENTED Require that a certain domain name (A record) be resolvable. Great for intranet domain names to prevent cloud sandboxing. Calls exit(0) if check fails.
	/// </summary>
	/// <param name="domain">The domain name to attempt to resolve</param>
	/// <returns></returns>
	void requireResolvableDomainName(char* domain);

	/// <summary>
	/// Require that the process is not running in Wine. Calls exit(0) if check fails.
	/// </summary>
	/// <returns></returns>
	void requireNotWine();

	/// <summary>
	/// Require that the process is not executing on virtualbox. Calls exit(0) if check fails.
	/// </summary>
	/// <returns></returns>
	void requireNotVbox();

	/// <summary>
	/// Require that a process is not executing in Vmware. Calls exit(0) if check fails.
	/// </summary>
	/// <returns></returns>
	void requireNotVmware();

	/// <summary>
	/// Require that a process is not being sandboxed by QEMU
	/// </summary>
	void requireNotQemu();

	/// <summary>
	/// Require that a process does not have a debugger attached. Calls exit(0) if check fails.
	/// </summary>
	/// <returns></returns>
	void requireNoDebuggerAttached();

	/// <summary>
	/// Require that a certain file exists. Calls exit(0) if check fails
	/// </summary>
	/// <param name="fileAbsolutePath">The filepath to check</param>
	void requireFileExists(char* fileAbsolutePath);

	/// <summary>
	/// Require that a certain directory exists. Calls exit(0) if check fails
	/// </summary>
	/// <param name="directoryAbsolutePath">The directory path to check</param>
	void requireDirectoryExists(char* directoryAbsolutePath);

	/// <summary>
	/// Require that common MS office applications exist (Word, Powerpoint, Excel, Outlook) are installed. Calls exit(0) if check fails.
	/// </summary>
	void requireCommonMsOfficeApplicationsInstalled();

	/// <summary>
	/// Require that there be mouse activity within the given duration. Calls exit(0) if check fails. 
	/// </summary>
	/// <param name="duration">The amount of time in seconds to wait for mouse activity before failing the check</param>
	void requireMouseActivity(int duration);

	/// <summary>
	/// Requires that the process isn't running under one of the following usernames: MALWARE, VIRUS, SANDBOX (case-adjusted). Calls exit(0) if check fails.
	/// </summary>
	void requireStdUsernameCheck();

	/// <summary>
	/// Require that the path doesn't contain obvious indicators like SAMPLE, SANDBOX, VIRUS. Calls exit(0) if check fails. 
	/// </summary>
	void requireStdPathCheck();

	/// <summary>
	/// Require that the hard disk for C:\ has the specified minimum size and miniumu free size. Note that values are rounded down.
	/// For example, if a disk has 49.9 GB, the check will assume it has 49 due to limitations of bit shift arithmetic.
	/// Calls exit(0) if check fails. 
	/// </summary>
	/// <param name="minDiskSizeGb">The minimum disk size in GB to allow without exiting</param>
	/// <param name="minFreeDiskSizeGb">The minimum free disk space to allow without exiting </param>
	void requireStdDriveSizeCheck(int minDiskSizeGb, int minFreeDiskSizeGb);

	/// <summary>
	/// Require that the sleep command isn't patched, which commonly happens in a sandbox. Calls exit(0) if check fails. 
	/// </summary>
	void requireSleepIsNotPatched();

	/// <summary>
	/// Require that a target has a given number of processors to execute. Sanboxes and VMs are likely to only have one
	/// </summary>
	/// <param name="numProcessors">The minimum number of processors to require</param>
	void requireMinimumNumProcessors(int numProcessors);

	/// <summary>
	/// Require that the system has at least n bytes of memory. Default is 8. Calls exit(0) if check fails.
	/// </summary>
	/// <param name="n">The minimum amount of memory in Gb</param>
	void requireMinimumGbMemory(int n = 8);

	/// <summary>
	/// Require that the system has been up for a certain amount of time. Calls exit(0) if check fails
	/// </summary>
	/// <param name="minutes">The amount of minutes. default is 12.</param>
	void requireSystemUptime(int minutes=12);
	
	/// <summary>
	/// Require that a printer is installed. Calls exit(0) if the check fails. 
	/// </summary>
	/// <param name="printerSubstring">Part or all of the name of the printer to check for, case-sensitive</param>
	void requirePrinterInstalled(string printerSubstring);

	/// <summary>
	/// Require that the process is not running in the Bochs hardware emulator. Calls exit(0) if the check fails.
	/// </summary>
	void requireNotBochsEmulator();
}
