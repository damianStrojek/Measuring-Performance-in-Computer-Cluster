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
#include <cstring>
#include <stdexcept>
#include <algorithm>
#include <sstream>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctime>
#include <iomanip>
#include <chrono>
#include <map>

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
	float memoryUsed;				// RAM used
	float memoryCached;				// Cache for files read from disk
	float swapUsed;					// Swap memory used
	float swapCached;				// Data previously written from memory to disk,
									// fetched back and still in the swap file
	float memoryActive;				// Data used in the last period
	float memoryInactive;			// Data used before memoryActive
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
	/*
	std::cout << "\nInterrupt Rate = " << systemMetrics.interruptRate << "\nContext switch rate = " << 
	systemMetrics.contextSwitchRate << "\nAll processes = " << systemMetrics.processesAll << 
	"\nRunning processes = " << systemMetrics.processesRunning << "\nBlocked processes = " << 
	systemMetrics.processesBlocked << "\n";
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

	stream >> temp;		// Get rid of 'cpu' at the beggining
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
	/*
	std::cout << "\nTime user = " << processorMetrics.timeUser  << "\nTime nice = " << processorMetrics.timeNice <<
	"\nTime system = " << processorMetrics.timeSystem << "\nTime idle = " << processorMetrics.timeIdle << 
	"\nTime I/O wait = " << processorMetrics.timeIoWait << "\nTime IRQ = " << processorMetrics.timeIRQ << 
	"\nTime Soft IRQ = " << processorMetrics.timeSoftIRQ << "\nTime Steal = " << processorMetrics.timeSteal << 
	"\nTime Guest = " << processorMetrics.timeGuest << "\n";
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
	/*
	std::cout << "\nI/O Read Rate = " << inputOutputMetrics.readRate << "\nI/O Write Rate = " << 
	inputOutputMetrics.writeRate << "\nI/O Read Operations Rate = " << inputOutputMetrics.readOperationsRate << 
	"\nI/O Write Operations Rate = " << inputOutputMetrics.writeOperationsRate << "\n";
	*/
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
	/*
	std::cout << "\nMemory Used = " << memoryMetrics.memoryUsed << " MB\nMemory Cached = " << memoryMetrics.memoryCached <<
	" MB\nSwap Used = " << memoryMetrics.swapUsed << " MB\nSwap Cached = " << memoryMetrics.swapCached << " MB\nMemory Active = " <<
	memoryMetrics.memoryActive << " MB\nMemory Inactive = " << memoryMetrics.memoryInactive << " MB\n";
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

	command = "cat /proc/net/dev | awk '/^ *eth0:/ {rx=$3; tx=$11; print rx,tx; exit}'";
	output = exec(command);
	std::stringstream streamTwo(output);

	streamTwo >> temp;
	networkMetrics.receivedData = std::stoi(temp);
	streamTwo >> temp;
	networkMetrics.sentData = std::stoi(temp);

	/// DEBUG
	/*
	std::cout << "\nReceive Packet Rate = " << networkMetrics.receivePacketRate << " KB/s\nSend Packet Rate = " << networkMetrics.sendPacketsRate << 
	" KB/s\nPackets Received = " <<  networkMetrics.receivedData << "\nPackets Sent = " << networkMetrics.sentData << "\n";
	*/
};

void writeToFileSystemMetrics(std::ofstream &file, SystemMetrics data) {
	file << (data.processesRunning != NOTSUPPORTED ? std::to_string(data.processesRunning) : "not_supported") << ",";
	file << (data.processesAll != NOTSUPPORTED ? std::to_string(data.processesAll) : "not_supported") << ",";
	file << (data.processesBlocked != NOTSUPPORTED ? std::to_string(data.processesBlocked) : "not_supported") << ",";
	file << (data.contextSwitchRate != NOTSUPPORTED ? std::to_string(data.contextSwitchRate) : "not_supported") << ",";
	file << (data.interruptRate != NOTSUPPORTED ? std::to_string(data.interruptRate) : "not_supported");
};

void writeToFileProcessorMetrics(std::ofstream &file, ProcessorMetrics data) {
	file << (data.timeUser != NOTSUPPORTED ? std::to_string(data.timeUser) : "not_supported") << ",";
	file << (data.timeNice != NOTSUPPORTED ? std::to_string(data.timeNice) : "not_supported") << ",";
	file << (data.timeSystem != NOTSUPPORTED ? std::to_string(data.timeSystem) : "not_supported") << ",";
	file << (data.timeIdle != NOTSUPPORTED ? std::to_string(data.timeIdle) : "not_supported") << ",";
	file << (data.timeIoWait != NOTSUPPORTED ? std::to_string(data.timeIoWait) : "not_supported") << ",";
	file << (data.timeIRQ != NOTSUPPORTED ? std::to_string(data.timeIRQ) : "not_supported") << ",";
	file << (data.timeSoftIRQ != NOTSUPPORTED ? std::to_string(data.timeSoftIRQ) : "not_supported") << ",";
	file << (data.timeSteal != NOTSUPPORTED ? std::to_string(data.timeSteal) : "not_supported") << ",";
	file << (data.timeGuest != NOTSUPPORTED ? std::to_string(data.timeGuest) : "not_supported") << ",";
	file << (data.timeGuestNice != NOTSUPPORTED ? std::to_string(data.timeGuestNice) : "not_supported") << ",";
	file << (data.instructionsRetiredRate != NOTSUPPORTED ? std::to_string(data.instructionsRetiredRate) : "not_supported") << ",";
	file << (data.cyclesRate != NOTSUPPORTED ? std::to_string(data.cyclesRate) : "not_supported") << ",";
	file << (data.cyclesReferenceRate != NOTSUPPORTED ? std::to_string(data.cyclesReferenceRate) : "not_supported") << ",";
	file << (data.frequencyRelative != NOTSUPPORTED ? std::to_string(data.frequencyRelative) : "not_supported") << ",";
	file << (data.frequencyActiveRelative != NOTSUPPORTED ? std::to_string(data.frequencyActiveRelative) : "not_supported") << ",";
	file << (data.cacheL2HitRate != NOTSUPPORTED ? std::to_string(data.cacheL2HitRate) : "not_supported") << ",";
	file << (data.cacheL2MissRate != NOTSUPPORTED ? std::to_string(data.cacheL2MissRate) : "not_supported") << ",";
	file << (data.cacheL3HitRate != NOTSUPPORTED ? std::to_string(data.cacheL3HitRate) : "not_supported") << ",";
	file << (data.cacheL3HitSnoopRate != NOTSUPPORTED ? std::to_string(data.cacheL3HitSnoopRate) : "not_supported") << ",";
	file << (data.cacheL3MissRate != NOTSUPPORTED ? std::to_string(data.cacheL3MissRate) : "not_supported") << ",";
	file << (data.processorPower != NOTSUPPORTED ? std::to_string(data.processorPower) : "not_supported") << ",";
};

void writeToFileInputOutputMetrics(std::ofstream &file, InputOutputMetrics data) {
	file << (data.readRate != NOTSUPPORTED ? std::to_string(data.readRate) : "not_supported") << ",";
	file << (data.readTime != NOTSUPPORTED ? std::to_string(data.readTime) : "not_supported") << ",";
	file << (data.readOperationsRate != NOTSUPPORTED ? std::to_string(data.readOperationsRate) : "not_supported") << ",";
	file << (data.writeRate != NOTSUPPORTED ? std::to_string(data.writeRate) : "not_supported") << ",";
	file << (data.writeTime != NOTSUPPORTED ? std::to_string(data.writeTime) : "not_supported") << ",";
	file << (data.writeOperationsRate != NOTSUPPORTED ? std::to_string(data.writeOperationsRate) : "not_supported") << ",";
	file << (data.flushTime != NOTSUPPORTED ? std::to_string(data.flushTime) : "not_supported") << ",";
	file << (data.flushOperationsRate != NOTSUPPORTED ? std::to_string(data.flushOperationsRate) : "not_supported") << ",";
};

void writeToFileMemoryMetrics(std::ofstream &file, MemoryMetrics data) {
	file << (data.memoryUsed != NOTSUPPORTED ? std::to_string(data.memoryUsed) : "not_supported") << ",";
	file << (data.memoryCached != NOTSUPPORTED ? std::to_string(data.memoryCached) : "not_supported") << ",";
	file << (data.swapUsed != NOTSUPPORTED ? std::to_string(data.swapUsed) : "not_supported") << ",";
	file << (data.swapCached != NOTSUPPORTED ? std::to_string(data.swapCached) : "not_supported") << ",";
	file << (data.memoryActive != NOTSUPPORTED ? std::to_string(data.memoryActive) : "not_supported") << ",";
	file << (data.memoryInactive != NOTSUPPORTED ? std::to_string(data.memoryInactive) : "not_supported") << ",";
	file << (data.pageInRate != NOTSUPPORTED ? std::to_string(data.pageInRate) : "not_supported") << ",";
	file << (data.pageOutRate != NOTSUPPORTED ? std::to_string(data.pageOutRate) : "not_supported") << ",";
	file << (data.pageFaultRate != NOTSUPPORTED ? std::to_string(data.pageFaultRate) : "not_supported") << ",";
	file << (data.pageFaultsMajorRate != NOTSUPPORTED ? std::to_string(data.pageFaultsMajorRate) : "not_supported") << ",";
	file << (data.pageFreeRate != NOTSUPPORTED ? std::to_string(data.pageFreeRate) : "not_supported") << ",";
	file << (data.pageActivateRate != NOTSUPPORTED ? std::to_string(data.pageActivateRate) : "not_supported") << ",";
	file << (data.pageDeactivateRate != NOTSUPPORTED ? std::to_string(data.pageDeactivateRate) : "not_supported") << ",";
	file << (data.memoryReadRate != NOTSUPPORTED ? std::to_string(data.memoryReadRate) : "not_supported") << ",";
	file << (data.memoryWriteRate != NOTSUPPORTED ? std::to_string(data.memoryWriteRate) : "not_supported") << ",";
	file << (data.memoryIoRate != NOTSUPPORTED ? std::to_string(data.memoryIoRate) : "not_supported") << ",";
	file << (data.memoryPower != NOTSUPPORTED ? std::to_string(data.memoryPower) : "not_supported") << ",";
};

void writeToFileNetworkMetrics(std::ofstream &file, NetworkMetrics data) {
	file << ((data.receiveRate == NOTSUPPORTED) ? "not_supported," : (std::to_string(data.receiveRate) + ","));
	file << ((data.receivePacketRate == NOTSUPPORTED) ? "not_supported," : (std::to_string(data.receivePacketRate) + ","));
	file << ((data.sendRate == NOTSUPPORTED) ? "not_supported," : (std::to_string(data.sendRate) + ","));
	file << ((data.sendPacketsRate == NOTSUPPORTED) ? "not_supported," : (std::to_string(data.sendPacketsRate) + ","));
};

void writeToCSV(std::string timestamp, SystemMetrics systemMetrics, ProcessorMetrics processorMetrics, 
				InputOutputMetrics inputOutputMetrics, MemoryMetrics memoryMetrics, NetworkMetrics networkMetrics){
    
	std::string fileName = timestamp += "_metrics.csv";
	std::ofstream file(fileName, std::ios::trunc);

    if (file.is_open()) {
		file << timestamp << ",";
        writeToFileSystemMetrics(file, systemMetrics);
		writeToFileProcessorMetrics(file, processorMetrics);
		writeToFileMemoryMertics(file, memoryMetrics);
		writeToFileInputOutputMetrics(file, inputOutputMetrics);
		writeToFileNetworkMetrics(file, networkMetrics);
		file << std::endl;
        file.close();
    } else
        std::cerr << "\n [ERROR] Unable to open file " << fileName << " for writing.\n";
};

void printMetric(std::string metricName, int metricValue, std::string metricUnit){
	std::string value = std::to_string(metricValue);
	if(metricValue == -692137) {
		value="notSupported";
		metricUnit="";
	}
	std::cout << metricName << std::right << std::setfill('.') << std::setw(30-metricName.length()) << value << metricUnit << std::endl;
}

void printMetricPair(std::string metricName, int metricValue, std::string metricUnit, std::string metricName2, int metricValue2, std::string metricUnit2){
	std::string value = std::to_string(metricValue);
	if(metricValue == -692137) {
		value="notSupported";
		metricUnit="";
	}
	std::string value2 = std::to_string(metricValue2);
	if(metricValue2 == -692137) {
		value2="notSupported";
		metricUnit2="";
	}
	std::cout << metricName << std::right << std::setfill('.') << std::setw(30-metricName.length()) << value << metricUnit;
	for(int i=0; i<20-metricUnit.length(); i++){
		std::cout << ' ';
	}
	std::cout << std::left << metricName2 << std::right << std::setfill('.') << std::setw(30-metricName2.length()) << value2 << metricUnit2 << std::endl;
}

void printMetrics(SystemMetrics* systemMetrics, ProcessorMetrics* processorMetrics, InputOutputMetrics* inputOutputMetrics, MemoryMetrics* memoryMetrics, NetworkMetrics* networkMetrics){
	auto now = std::chrono::system_clock::now();
  	std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::cout << std::put_time(std::localtime(&now_c), "%Y-%m-%d %X") << '\n';
	std::cout << "System:";
	for(int i=0;i<43;i++){
		std::cout << ' ';
	}
	std::cout << "Network:" << '\n';
	printMetricPair("Processes running", systemMetrics->processesRunning,"", "Received packets",networkMetrics->receivedData,"");
	printMetricPair("All Processes", systemMetrics->processesAll,"","Received packets rate", networkMetrics->receivePacketRate,"KB/s");
	printMetricPair("Context Switch Rate", systemMetrics->contextSwitchRate,"/s","Sent packets",networkMetrics->sentData,"");
	printMetricPair("Interrupt rate", systemMetrics->interruptRate,"/s","Sent packets rate",networkMetrics->sendPacketsRate,"KB/s");
	std::cout << '\n';
	std::cout << "Memory:";
	for(int i=0;i<43;i++){
		std::cout << ' ';
	}
	std::cout << "Processor:" << '\n';
	printMetricPair("Memory used", memoryMetrics->memoryUsed,"MB","Time user",processorMetrics->timeUser,"USER_HZ");
	printMetricPair("Memory cached", memoryMetrics->memoryCached,"MB","Time system",processorMetrics->timeSystem,"USER_HZ");
	printMetricPair("Swap used", memoryMetrics->swapUsed,"MB","Time idle",processorMetrics->timeIdle,"USER_HZ");
	printMetricPair("Swap cached", memoryMetrics->swapCached,"MB","Time I/O wait",processorMetrics->timeIoWait,"USER_HZ");
	printMetricPair("Memory Active", memoryMetrics->memoryActive,"MB","Time IRQ",processorMetrics->timeIRQ,"USER_HZ");
	printMetricPair("Memory Inactive", memoryMetrics->memoryInactive,"MB","Time Steal",processorMetrics->timeSteal,"USER_HZ");
	std::cout << '\n'<< "I/O" << '\n';
	printMetric("Read Rate",inputOutputMetrics->readRate,"MB/s");
	printMetric("Write Rate",inputOutputMetrics->writeRate,"MB/s");
	printMetric("Read operations rate",inputOutputMetrics->readOperationsRate,"/s");
	printMetric("Write operations rate",inputOutputMetrics->writeOperationsRate,"/s");
	std::cout << '\n';
}

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

	while(true){
    	if(keyboardHit()){
			std::cout << "\n\nKey pressed, STOPPING LOOP.\n\n";
			break;
		}
		
		//auto start = std::chrono::high_resolution_clock::now();

	const char* command = "date +'%d%m%y-%H%M%S'";
	std::string timestamp;

	while(true) {
    	if(keyboardHit()) {
			std::cout << "\n\n [STOP] Key pressed.\n\n";
			break;
		}	
    
		timestamp = exec(command);
		std::cout << "\n\n [TIMESTAMP] " << timestamp << "\n";

		getSystemMetrics(systemMetrics);
		getProcessorMetrics(processorMetrics);
		getInputOutputMetrics(inputOutputMetrics);
		getMemoryMetrics(memoryMetrics);
		getNetworkMetrics(networkMetrics);

		//auto end = std::chrono::high_resolution_clock::now();
		//auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end-start);
		//std::cout<<"Time taken to get all measures:" << duration.count()<< "microseconds" << '\n';

		// Display metrics
		printMetrics(&systemMetrics, &processorMetrics, &inputOutputMetrics, &memoryMetrics, &networkMetrics);

		// Save metrics to file
	  writeToCSV(timestamp, systemMetrics, processorMetrics, inputOutputMetrics, memoryMetrics, networkMetrics);

		sleep(1);
  	}
    
	return 0;
};