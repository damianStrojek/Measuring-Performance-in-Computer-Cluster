// Gdansk University of Technology 2023
// Group project nr 8 realised at WETI KASK
// Title: An application for monitoring performance and energy consumption in a computing cluster
// Authors: Damian Strojek, Piotr Garbowski, Jakub Wasniewski
#include <iostream>
#include <string>
#include <fstream>
#include <cstdio>
#include <array>
#include <memory>
#include <stdexcept>
#include <algorithm>
#include <sstream>

// Constant paths to files in Linux that contain information about different metrics
#define LOADAVG "/proc/loadavg"			// Number of tasks in the queue, process units that are being executed
#define CPUINFO "/proc/cpuinfo"			// Detailed information about each CPU in the system (numbered from 0)
#define PROCSTAT "/proc/stat"			// The amount of time CPU spent performing various types of work
#define INTERRUPTS "/proc/interrupts"	// Counting each system interrupt
#define PROCIO "/proc/$$/io"			// Information about the given task (in this case, task $$).
										// For example: the number of bytes written to and read from memory
#define MEMINFO "/proc/meminfo"			// Operating memory information
#define SWAPINFO "/proc/swap"			// List of the available swap areas

/*
EXAMPLE OF GETTING THE FILE INTO A STRING

std::ifstream procLoadavg;
	procLoadavg.open(LOADAVG, std::ios::in);
	if (procLoadavg.is_open()) {
		std::string loadAvg((std::istreambuf_iterator<char>(procLoadavg)),
			std::istreambuf_iterator<char>());
		std::cout << loadAvg << "\n";
	}
	else std::cout << "\n[ERROR] provLoadAvg not opened\n";
*/

struct SystemMetrics {
	int processesRunning;			// Number of processes in the R state
	int processesAll;				// Number of all processes
	int processesBlocked;			// Number of processes waiting for I/O operation to complete
	int contextSwitchRate;			// Number of context switches per second
	int interruptRate;				// Number of all interrupts handled per second
};

struct ProcessorMetrics {
	int timeUser;					// Time spent in user space
	int timeNice;					// Time spent in user with low priority space
	int timeSystem;					// Time spent in system space
	int timeIdle;					// Time spent on idle task
	int timeIoWait;					// Time spent waiting for I/O operation to complete
	int timeIRQ;					// Interrupt handling time
	int timeSoftIRQ;				// SoftIRQ handling time
	int timeSteal;					// Time spent in other OSs in visualization mode
	int timeGuest;					// Virtual CPU uptime for other OSs under kernel control
	int timeGuestNice;				// Virtual CPU (low priority) uptime for other OSs under kernel control
	int instructionsRetiredRate;	// Number of actualy executed instructions
	int cyclesRate;					// Number of clock cycles during core operation (Turbo Boost possible)
	int cyclesReferenceRate;		// Number of reference clock cycles
	int frequencyRelative;			// Average core clock frequency, also taking into account Turbo Boost
	int frequencyActiveRelative;	// Average core clock frequency if not in C0 state, also including TB
	int cacheL2HitRate;				// Number of L2 cache hits
	int cacheL2MissRate;			// Number of L2 cache misses
	int cacheL3HitRate;				// Number of L3 cache hits
	int cacheL3HitSnoopRate;		// Number of L3 cache hits, with references to sibling L2
	int cacheL3MissRate;			// Number of L3 cache misses
	int processorPower;				// Power consumed by the processor
};

struct MemoryMetrics {
	// Amount of memory is counted in MB
	int memoryUsed;					// RAM used
	int memoryCached;				// Cache for files read from disk
	int swapUsed;					// Swap memory used
	int swapCached;					// Data previously written from memory to disk,
									// fetched back and still in the swap file
	int memoryActive;				// Data used in the last period
	int memoryInactive;				// Data used before memoryActive
	// Number of pages is given in pages/second
	int pageInRate;					// Pages read
	int pageOutRate;				// Pages saved
	int pageFaultRate;				// No page status
	int pageFaultsMajorRate;		// Page missing (need to load from disk)
	int pageFreeRate;				// Page release
	int pageActivateRate;			// Page activation
	int pageDeactivateRate;			// Page deactivation
	// Read, write, and io rate are measured in MB/s
	int memoryReadRate;				// Reading from memory
	int memoryWriteRate;			// Writing to memory
	int memoryIoRate;				// Requests to read/write data from all I/O devices
	int memoryPower;				// Power consumed by memory
};

struct NetworkMetrics {
	int receiveRate;				// Data received
	int receivePacketRate;			// Packets received
	int sendRate;					// Data sent
	int sendPacketsRate;			// Packets sent
};

// Execute system command and return the output in the string
std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe)
        throw std::runtime_error("popen() failed!");
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
        result += buffer.data();
    return result;
}

// Get all of the system metrics into structure
void getSystemMetrics(SystemMetrics &systemMetrics) {

	// Interrupt rate and context switch rate from vmstat command
	const char* command = "vmstat";
	std::string output = exec(command), temp;
	temp = output.substr(219, 4);
	systemMetrics.interruptRate = std::stoi(temp);
	temp = output.substr(224, 4);
	systemMetrics.contextSwitchRate = std::stoi(temp); 
	
	// All and running processes from /proc/loadavg file
	command = "cat /proc/loadavg | cut -d ' ' -f 4";
	output = exec(command);
	size_t slashPosition = output.find('/');
	temp = output.substr(0, slashPosition);
	systemMetrics.processesRunning = std::stoi(temp);
	temp = output.substr(slashPosition+1);
	systemMetrics.processesAll = std::stoi(temp);

	// Blocked processes from ps command
	command = "ps -eo state | grep -c '^D'";
	output = exec(command);
	systemMetrics.processesBlocked = std::stoi(output);
};

void getProcessorMetrics(ProcessorMetrics &processorMetrics){

	// Number of processors that were reported to /proc/cpuinfo
	const char* command = "cat /proc/cpuinfo | grep 'processor' -c";
	std::string output = exec(command), temp;
	int numberOfProcessors = std::stoi(output);

	// Sum of information about processors
	command = "cat /proc/stat";
	output = exec(command);
	std::stringstream stream(output);
	stream >> temp;			// Get rid of 'cpu' at the beggining
	stream >> temp;
	processorMetrics.timeUser = std::stoi(temp);
	stream >> temp;
	processorMetrics.timeNice = std::stoi(temp);
	stream >> temp;
	processorMetrics.timeSystem = std::stoi(temp);
	stream >> temp;
	processorMetrics.timeIdle = std::stoi(temp);
	stream >> temp;
	processorMetrics.timeIoWait = std::stoi(temp);
	stream >> temp;
	processorMetrics.timeIRQ = std::stoi(temp);
	stream >> temp;
	processorMetrics.timesoftIRQ = std::stoi(temp);
	stream >> temp;
	processorMetrics.timeSteal= std::stoi(temp);
	stream >> temp;
	processorMetrics.timeGuest = std::stoi(temp);

	/* If we ever wanted to do it for each processor
	for(int i = 0; i < numberOfProcessors; i++){
		std::getline(stream, temp);
		std::cout << temp;
	}
	*/
};

int main() {
	SystemMetrics systemMetrics;
	ProcessorMetrics processorMetrics;

	getSystemMetrics(systemMetrics);
	getProcessorMetrics(processorMetrics);

	// DEBUG
	std::cout << "\n Interrupt Rate = " << systemMetrics.interruptRate 
		<< " Context switch rate = " << systemMetrics.contextSwitchRate << " "
		<< " All processes = " << systemMetrics.processesAll << " "
		<< " Running processes = " << systemMetrics.processesRunning << " "
		<< " Blocked processes = " << systemMetrics.processesBlocked;
	return 0;
}