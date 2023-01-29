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

struct InputOutputMetrics {
	int readRate;					// Data read
	int readTime;					// Data read time
	int readOperationsRate;			// Amount of read operations per second
	int writeRate;					// Data written
	int writeTime;					// Data write time
	int writeOperationsRate;		// Amount of write operations per second
	int flushTime;					// Flush execution time
	int flushOperationsRate;		// Amount of flush operations per second
};

struct MemoryMetrics {
	// Amount of memory is counted in MB
	float memoryUsed;					// RAM used
	float memoryCached;				// Cache for files read from disk
	float swapUsed;					// Swap memory used
	float swapCached;					// Data previously written from memory to disk,
									// fetched back and still in the swap file
	float memoryActive;				// Data used in the last period
	float memoryInactive;				// Data used before memoryActive
	// Number of pages is given in pages/second
	int pageInRate;					// Pages read
	int pageOutRate;				// Pages saved
	int pageFaultRate;				// No page status
	int pageFaultsMajorRate;		// Page missing (need to load from disk)
	int pageFreeRate;				// Page release
	int pageActivateRate;			// Page activation
	int pageDeactivateRate;			// Page deactivation
	// Read, write, and io rate are measured in MB/s
	float memoryReadRate;				// Reading from memory
	float memoryWriteRate;			// Writing to memory
	float memoryIoRate;				// Requests to read/write data from all I/O devices
	int memoryPower;				// Power consumed by memory
};

struct NetworkMetrics {
	float receiveRate;				// Data received
	float receivePacketRate;		// Packets received
	float sendRate;					// Data sent
	float sendPacketsRate;			// Packets sent
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
};

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

	/* // DEBUG
	std::cout << "Interrupt Rate = " << systemMetrics.interruptRate 
		<< " Context switch rate = " << systemMetrics.contextSwitchRate
		<< " All processes = " << systemMetrics.processesAll
		<< " Running processes = " << systemMetrics.processesRunning
		<< " Blocked processes = " << systemMetrics.processesBlocked << "\n";
	*/
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

	stream >> temp;				// Get rid of 'cpu' at the beggining
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
	processorMetrics.timeSoftIRQ = std::stoi(temp);
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

	command = "perf stat -e L2_cache_references,L2_cache_misses,L3_cache_references,L3_cache_misses,L3_cache_snoop_hits,L3_cache_snoop_misses";
	output = exec(command);
	std::stringstream stream(output);
	int references, snoopHits, snoopMisses;

	stream >> temp;
	references = std::stoi(temp);
	stream >> temp;
	processorMetrics.cacheL2MissRate = std::stoi(temp);
	processorMetrics.cacheL2HitRate = references / (references + processorMetrics.cacheL2MissRate);

	stream >> temp;
	references = std::stoi(temp);
	stream >> temp;
	processorMetrics.cacheL3MissRate = std::stoi(temp);
	processorMetrics.cacheL3HitRate = references / (references + processorMetrics.cacheL3MissRate);

	stream >> temp;
	snoopHits = std::stoi(temp);
	stream >> temp;
	snoopMisses = std::stoi(temp);
	processorMetrics.cacheL3HitSnoopRate = snoopHits / (snoopHits + snoopMisses);

	/*
	TO DO
	int timeGuestNice;				// Virtual CPU (low priority) uptime for other OSs under kernel control
	int instructionsRetiredRate;	// Number of actualy executed instructions
	int cyclesRate;					// Number of clock cycles during core operation (Turbo Boost possible)
	int cyclesReferenceRate;		// Number of reference clock cycles
	int frequencyRelative;			// Average core clock frequency, also taking into account Turbo Boost
	int frequencyActiveRelative;	// Average core clock frequency if not in C0 state, also including TB
	int processorPower;				// Power consumed by the processor
	*/

	/* // DEBUG
	std::cout << "Time user = " << processorMetrics.timeUser << " Time nice = " << processorMetrics.timeNice <<
	" Time system = " << processorMetrics.timeSystem << " Time idle = " << processorMetrics.timeUser << 
	" Time IO Wait = " << processorMetrics.timeIoWait << " Time IRQ = " << processorMetrics.timeIRQ <<
	" Time soft IRQ = " << processorMetrics.timeSoftIRQ << " Time Steal = " << processorMetrics.timeSteal <<
	" Time Guest = " << processorMetrics.timeGuest << "\n";
	*/
};

void getInputOutputMetrics(InputOutputMetrics &inputOutputMetrics){
	
	const char* command = "awk '{ print $2 }' /proc/$$/io";
	std::string output = exec(command), temp;
	std::stringstream stream(output);
	
	stream >> temp;
	inputOutputMetrics.readRate = std::stoi(temp);
	stream >> temp;
	inputOutputMetrics.writeRate = std::stoi(temp);
	stream >> temp;
	inputOutputMetrics.readOperationsRate = std::stoi(temp);
	stream >> temp;
	inputOutputMetrics.writeOperationsRate = std::stoi(temp);
	stream >> temp;

	command = "perf stat -e io:r,io:w,io:f -I 1000";
	output = exec(command);
	std::stringstream stream(output);

	stream >> temp;
	inputOutputMetrics.readTime = std::stoi(temp);
	stream >> temp;
	inputOutputMetrics.writeTime = std::stoi(temp);
	stream >> temp;
	inputOutputMetrics.flushOperationsRate = std::stoi(temp);
	// Tutaj ma trafić czas wykonywania tego polecenia
	stream >> temp;
	inputOutputMetrics.flushTime = inputOutputMetrics.flushOperationsRate / std::stoi(temp);

	/* // DEBUG
	std::cout << "Read Rate = " << inputOutputMetrics.readRate << " Write Rate = " << inputOutputMetrics.writeRate <<
	" Read Operations Rate = " << inputOutputMetrics.readOperationsRate << " Write Operations Rate = " <<
	inputOutputMetrics.writeOperationsRate << "\n";
	*/
};

void getMemoryMetrics(MemoryMetrics &memoryMetrics){

	const char* command = "cat /proc/meminfo";
	std::string ouput = exec(command), temp;

	stream >> temp;
	memoryMetrics.memoryUsed =		// MemTotal - MemFree
	stream >> temp;
	memoryMetrics.memoryCached =	// Cached
	stream >> temp;
	memoryMetrics.swapUsed =		// SwapTotal - SwapFree
	stream >> temp;
	memoryMetrics.swapCached =		// Swap Cached
	stream >> temp;
	memoryMetrics.memoryActive =	// Active
	stream >> temp;
	memoryMetrics.memoryInactive =	// Inactive

	// Page Input Rate, Page Output Rate, Page Fault Rate, Page Free Rate, Page Activate Rate, Page Deactivate Rate
	command = "vmstat 1 2 | awk 'NR==3{print '$7', '$8', '$9', '$10', '$11', '$12'}'";
	
	
	// Read Rate, Write Rate, I/O Read requests, I/O Write requests
	command = "iostat -x 1 2 | awk 'NR==4{print '$4', '$5', '$6', '$7'}'";


	/*
	TO DO
	int memoryPower;				// Power consumed by memory
	*/

	/* // DEBUG
	std::cout << "Memory Used = " << memoryMetrics.memoryUsed << "\nMemory Cached = " << memoryMetrics.memoryCached <<
	"\nSwap Used = " << memoryMetrics.swapUsed << "\nSwap Cached = " << memoryMetrics.swapCached << "\nMemory Active = " <<
	memoryMetrics.memoryActive << "\nMemory Inactive" << memoryMetrics.memoryInactive << 
	
	
	"\nMemory Read Rate = " << << "\nMemory Write Rate = " << << "\nI/O Read Requests = " << << "\nI/O Write Requests" << << 
	*/
};

void getNetworkMetrics(NetworkMetrics &networkMetrics){

	const char* command = "ifstat 1 1 | tail -1 | awk '{ print $1, $2 }'";
	std::string output = exec(command), temp;
	std::stringstream stream(output);

	stream >> temp;
	networkMetrics.receivePacketRate = std::stof(temp);
	stream >> temp;
	networkMetrics.sendPacketsRate = std::stof(temp);

	command = "ifconfig eth0 | awk '/RX bytes/{print '$2/1024/1024'}/TX bytes/{print '$10/1024/1024'}'";
	output = exec(command);
	std::stringstream stream(output);

	stream >> temp;
	networkMetrics.receiveRate = std::stof(temp);
	stream >> temp;
	networkMetrics.sendRate = std::stof(temp);

	/* // DEBUG
	std::cout << "Receive Packet Rate = " << networkMetrics.receivePacketRate << " Send Packet Rate = " << networkMetrics.sendPacketsRate << "\n";
	*/
};

int main() {

	SystemMetrics systemMetrics;
	ProcessorMetrics processorMetrics;
	InputOutputMetrics inputOutputMetrics;
	MemoryMetrics memoryMetrics;
	NetworkMetrics networkMetrics;

	getSystemMetrics(systemMetrics);
	getProcessorMetrics(processorMetrics);
	getInputOutputMetrics(inputOutputMetrics);
	getMemoryMetrics(memoryMetr)
	getNetworkMetrics(networkMetrics);
	
	return 0;
};