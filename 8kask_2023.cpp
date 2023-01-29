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
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#define NOTSUPPORTED -692137			// Functionality not yet supported

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
	float memoryReadRate;			// Reading from memory
	float memoryWriteRate;			// Writing to memory
	float memoryIoRate;				// Requests to read/write data from all I/O devices
	int memoryPower;				// Power consumed by memory
};

struct NetworkMetrics {
	int receivedData;				// All of the packets sent
	float receivePacketRate;		// packets that are being received in KB/s
	int sentData;					// All of the packets sent
	float sendPacketsRate;			// packets that are being sent in KB/s
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

	// DEBUG
	std::cout << "\nInterrupt Rate = " << systemMetrics.interruptRate << "\nContext switch rate = " << 
	systemMetrics.contextSwitchRate << "\nAll processes = " << systemMetrics.processesAll << 
	"\nRunning processes = " << systemMetrics.processesRunning << "\nBlocked processes = " << 
	systemMetrics.processesBlocked << "\n";
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
	processorMetrics.timeSteal = std::stoi(temp);
	stream >> temp;
	processorMetrics.timeGuest = std::stoi(temp);

	/* If we ever wanted to do it for each processor
	for(int i = 0; i < numberOfProcessors; i++){
		std::getline(stream, temp);
		std::cout << temp;
	}
	*/

	/*
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
	*/

	// Temporary values - don't know how to use perf to collect this information
	processorMetrics.cacheL2MissRate = NOTSUPPORTED;
	processorMetrics.cacheL2HitRate = NOTSUPPORTED;
	processorMetrics.cacheL3MissRate = NOTSUPPORTED;
	processorMetrics.cacheL3HitRate = NOTSUPPORTED;
	processorMetrics.cacheL3HitSnoopRate = NOTSUPPORTED;

	processorMetrics.instructionsRetiredRate = NOTSUPPORTED;
	processorMetrics.cyclesRate = NOTSUPPORTED;
	processorMetrics.cyclesReferenceRate = NOTSUPPORTED;
	processorMetrics.frequencyRelative = NOTSUPPORTED;
	processorMetrics.frequencyActiveRelative = NOTSUPPORTED;
	processorMetrics.processorPower = NOTSUPPORTED;

	// DEBUG
	std::cout << "\nTime user = " << processorMetrics.timeUser  << "\nTime nice = " << processorMetrics.timeNice <<
	"\nTime system = " << processorMetrics.timeSystem << "\nTime idle = " << processorMetrics.timeIdle << 
	"\nTime I/O wait = " << processorMetrics.timeIoWait << "\nTime IRQ = " << processorMetrics.timeIRQ << 
	"\nTime Soft IRQ = " << processorMetrics.timeSoftIRQ << "\nTime Steal = " << processorMetrics.timeSteal << 
	"\nTime Guest = " << processorMetrics.timeGuest << "\n";
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

	/*
	command = "perf stat -e io:r,io:w,io:f -I 1000";
	output = exec(command);
	std::stringstream streamTwo(output);

	streamTwo >> temp;
	inputOutputMetrics.readTime = std::stoi(temp);
	streamTwo >> temp;
	inputOutputMetrics.writeTime = std::stoi(temp);
	streamTwo >> temp;
	inputOutputMetrics.flushOperationsRate = std::stoi(temp);
	// Tutaj ma trafić czas wykonywania tego polecenia
	streamTwo >> temp;
	inputOutputMetrics.flushTime = inputOutputMetrics.flushOperationsRate / std::stoi(temp);
	*/

	// Temporary values
	inputOutputMetrics.readTime = NOTSUPPORTED;
	inputOutputMetrics.writeTime = NOTSUPPORTED;
	inputOutputMetrics.flushOperationsRate = NOTSUPPORTED;
	inputOutputMetrics.flushTime = NOTSUPPORTED;

	// DEBUG
	std::cout << "\nI/O Read Rate = " << inputOutputMetrics.readRate << "\nI/O Write Rate = " << 
	inputOutputMetrics.writeRate << "\nI/O Read Operations Rate = " << inputOutputMetrics.readOperationsRate << 
	"\nI/O Write Operations Rate = " << inputOutputMetrics.writeOperationsRate << "\n";
};

void getMemoryMetrics(MemoryMetrics &memoryMetrics){

	const char* command = "grep -v -e 'anon' -e 'file' /proc/meminfo | grep -E '^(Cached|SwapCached|SwapTotal|SwapFree|Active|Inactive)' | awk '{print $2}'";
	std::string output = exec(command), temp, swapFree;
	std::stringstream stream(output);

	stream >> temp;
	memoryMetrics.memoryCached = std::stof(temp);
	memoryMetrics.memoryCached = memoryMetrics.memoryCached / 1024;
	stream >> temp;
	memoryMetrics.swapCached = std::stof(temp);
	memoryMetrics.swapCached = memoryMetrics.swapCached / 1024;
	stream >> temp;
	memoryMetrics.memoryActive = std::stof(temp);
	memoryMetrics.memoryActive = memoryMetrics.memoryActive / 1024;
	stream >> temp;
	memoryMetrics.memoryInactive = std::stof(temp);
	memoryMetrics.memoryInactive = memoryMetrics.memoryInactive / 1024;
	stream >> temp;
	stream >> swapFree;
	memoryMetrics.swapUsed = std::stof(temp) - std::stof(swapFree);
	memoryMetrics.swapUsed = memoryMetrics.swapUsed / 1024;

	// Page Input Rate, Page Output Rate, Page Fault Rate, Page Free Rate, Page Activate Rate, Page Deactivate Rate
	// command = "vmstat 1 2 | awk 'NR==3{print '$7', '$8', '$9', '$10', '$11', '$12'}'";
	// Temporary values
	memoryMetrics.pageInRate = NOTSUPPORTED;
	memoryMetrics.pageOutRate = NOTSUPPORTED;
	memoryMetrics.pageFaultRate = NOTSUPPORTED;
	memoryMetrics.pageFreeRate = NOTSUPPORTED;
	memoryMetrics.pageActivateRate = NOTSUPPORTED;
	memoryMetrics.pageDeactivateRate = NOTSUPPORTED;
	
	// Read Rate, Write Rate, I/O Read requests, I/O Write requests
	// command = "iostat -x 1 2 | awk 'NR==4{print '$4', '$5', '$6', '$7'}'";
	// Temporary values
	memoryMetrics.memoryReadRate = NOTSUPPORTED;
	memoryMetrics.memoryWriteRate = NOTSUPPORTED;
	memoryMetrics.memoryIoRate = NOTSUPPORTED;
	memoryMetrics.memoryPower = NOTSUPPORTED;

	// DEBUG
	std::cout << "\nMemory Used = " << memoryMetrics.memoryUsed << " MB\nMemory Cached = " << memoryMetrics.memoryCached <<
	" MB\nSwap Used = " << memoryMetrics.swapUsed << " MB\nSwap Cached = " << memoryMetrics.swapCached << " MB\nMemory Active = " <<
	memoryMetrics.memoryActive << " MB\nMemory Inactive = " << memoryMetrics.memoryInactive << " MB\n";
};

void getNetworkMetrics(NetworkMetrics &networkMetrics){

	const char* command = "ifstat 1 1 | tail -1 | awk '{ print $1, $2 }'";
	std::string output = exec(command), temp;
	std::stringstream stream(output);

	stream >> temp;
	networkMetrics.receivePacketRate = std::stof(temp);
	stream >> temp;
	networkMetrics.sendPacketsRate = std::stof(temp);

	command = "cat /proc/net/dev | awk '/^ *eth0:/ {rx=$3; tx=$11; print rx,tx; exit}'";
	output = exec(command);
	std::stringstream streamTwo(output);

	streamTwo >> temp;
	networkMetrics.receivedData = std::stoi(temp);
	streamTwo >> temp;
	networkMetrics.sentData = std::stoi(temp);

	/// DEBUG
	std::cout << "\nReceive Packet Rate = " << networkMetrics.receivePacketRate << " KB/s\nSend Packet Rate = " << networkMetrics.sendPacketsRate << 
	" KB/s\nPackets Received = " <<  networkMetrics.receivedData << "\nPackets Sent = " << networkMetrics.sentData << "\n";
};

int keyboardHit(void) {
	struct termios oldt, newt;
	int ch;
	int oldf;

	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

	ch = getchar();

	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);

	if(ch != EOF) {
		ungetc(ch, stdin);
		return 1;
	}
	return 0;
};

int main() {

	// Structures for all metrics
	SystemMetrics systemMetrics;
	ProcessorMetrics processorMetrics;
	InputOutputMetrics inputOutputMetrics;
	MemoryMetrics memoryMetrics;
	NetworkMetrics networkMetrics;
	const char* command = "date +'%d%m%y-%H%M%S'";
	std::string timestamp;

	while(true){
    	if(keyboardHit()){
			std::cout << "\n\nKey pressed, STOPPING LOOP.\n\n";
			break;
		}	
		timestamp = exec(command);
		std::cout << "\n\n !!! " << timestamp << "\n";

		getSystemMetrics(systemMetrics);
		getProcessorMetrics(processorMetrics);
		getInputOutputMetrics(inputOutputMetrics);
		getMemoryMetrics(memoryMetrics);
		getNetworkMetrics(networkMetrics);

		// Display metrics

		// Save metrics to file

		sleep(3);
  	}

	return 0;
};