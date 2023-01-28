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
#include <map>

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
	float receivePacketRate;		// Packets received
	int sendRate;					// Data sent
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
};

void getInputOutputMetrics(InputOutputMetrics &inputOuputMetrics){
	
	// Not sure if this is working - I have no linux available rn
	// Have to check whether those metrics make any sense
	const char* command = "awk '{ print $2 }' /proc/$$/io";
	std::string output = exec(command), temp;
	
	std::stringstream stream(output);
	stream >> temp;
	inputOuputMetrics.readRate = std::stoi(temp);
	stream >> temp;
	inputOuputMetrics.writeRate = std::stoi(temp);
	stream >> temp;
	inputOuputMetrics.readOperationsRate = std::stoi(temp);
	stream >> temp;
	inputOuputMetrics.writeOperationsRate = std::stoi(temp);
	stream >> temp;

};

void getNetworkMetrics(NetworkMetrics &networkMetrics){

	const char* command = "ifstat 1 1 | tail -1 | awk '{ print $1, $2 }'";
	std::string output = exec(command), temp;

	std::stringstream stream(output);
	stream >> temp;
	networkMetrics.receivePacketRate = std::stof(temp);
	stream >> temp;
	networkMetrics.sendPacketsRate = std::stof(temp);

};

void writeToFileSystemMetrics(std::ofstream &file, SystemMetrics data) {
	file << (data.processesRunning != -213769 ? std::to_string(data.processesRunning) : "not_supported") << ",";
	file << (data.processesAll != -213769 ? std::to_string(data.processesAll) : "not_supported") << ",";
	file << (data.processesBlocked != -213769 ? std::to_string(data.processesBlocked) : "not_supported") << ",";
	file << (data.contextSwitchRate != -213769 ? std::to_string(data.contextSwitchRate) : "not_supported") << ",";
	file << (data.interruptRate != -213769 ? std::to_string(data.interruptRate) : "not_supported");

}
void writeToFileProcessorMetrics(std::ofstream &file, ProcessorMetrics data) {
	file << (data.timeUser != -213769 ? std::to_string(data.timeUser) : "not_supported") << ",";
	file << (data.timeNice != -213769 ? std::to_string(data.timeNice) : "not_supported") << ",";
	file << (data.timeSystem != -213769 ? std::to_string(data.timeSystem) : "not_supported") << ",";
	file << (data.timeIdle != -213769 ? std::to_string(data.timeIdle) : "not_supported") << ",";
	file << (data.timeIoWait != -213769 ? std::to_string(data.timeIoWait) : "not_supported") << ",";
	file << (data.timeIRQ != -213769 ? std::to_string(data.timeIRQ) : "not_supported") << ",";
	file << (data.timeSoftIRQ != -213769 ? std::to_string(data.timeSoftIRQ) : "not_supported") << ",";
	file << (data.timeSteal != -213769 ? std::to_string(data.timeSteal) : "not_supported") << ",";
	file << (data.timeGuest != -213769 ? std::to_string(data.timeGuest) : "not_supported") << ",";
	file << (data.timeGuestNice != -213769 ? std::to_string(data.timeGuestNice) : "not_supported") << ",";
	file << (data.instructionsRetiredRate != -213769 ? std::to_string(data.instructionsRetiredRate) : "not_supported") << ",";
	file << (data.cyclesRate != -213769 ? std::to_string(data.cyclesRate) : "not_supported") << ",";
	file << (data.cyclesReferenceRate != -213769 ? std::to_string(data.cyclesReferenceRate) : "not_supported") << ",";
	file << (data.frequencyRelative != -213769 ? std::to_string(data.frequencyRelative) : "not_supported") << ",";
	file << (data.frequencyActiveRelative != -213769 ? std::to_string(data.frequencyActiveRelative) : "not_supported") << ",";
	file << (data.cacheL2HitRate != -213769 ? std::to_string(data.cacheL2HitRate) : "not_supported") << ",";
	file << (data.cacheL2MissRate != -213769 ? std::to_string(data.cacheL2MissRate) : "not_supported") << ",";
	file << (data.cacheL3HitRate != -213769 ? std::to_string(data.cacheL3HitRate) : "not_supported") << ",";
	file << (data.cacheL3HitSnoopRate != -213769 ? std::to_string(data.cacheL3HitSnoopRate) : "not_supported") << ",";
	file << (data.cacheL3MissRate != -213769 ? std::to_string(data.cacheL3MissRate) : "not_supported") << ",";
	file << (data.processorPower != -213769 ? std::to_string(data.processorPower) : "not_supported") << ",";

}
void writeToFileInputOutputMetrics(std::ofstream &file, InputOutputMetrics data) {
	file << (data.readRate != -213769 ? std::to_string(data.readRate) : "not_supported") << ",";
	file << (data.readTime != -213769 ? std::to_string(data.readTime) : "not_supported") << ",";
	file << (data.readOperationsRate != -213769 ? std::to_string(data.readOperationsRate) : "not_supported") << ",";
	file << (data.writeRate != -213769 ? std::to_string(data.writeRate) : "not_supported") << ",";
	file << (data.writeTime != -213769 ? std::to_string(data.writeTime) : "not_supported") << ",";
	file << (data.writeOperationsRate != -213769 ? std::to_string(data.writeOperationsRate) : "not_supported") << ",";
	file << (data.flushTime != -213769 ? std::to_string(data.flushTime) : "not_supported") << ",";
	file << (data.flushOperationsRate != -213769 ? std::to_string(data.flushOperationsRate) : "not_supported") << ",";

}

void writeToFileMemoryMetrics(std::ofstream &file, MemoryMetrics data) {
	file << (data.memoryUsed != -213769 ? std::to_string(data.memoryUsed) : "not_supported") << ",";
	file << (data.memoryCached != -213769 ? std::to_string(data.memoryCached) : "not_supported") << ",";
	file << (data.swapUsed != -213769 ? std::to_string(data.swapUsed) : "not_supported") << ",";
	file << (data.swapCached != -213769 ? std::to_string(data.swapCached) : "not_supported") << ",";
	file << (data.memoryActive != -213769 ? std::to_string(data.memoryActive) : "not_supported") << ",";
	file << (data.memoryInactive != -213769 ? std::to_string(data.memoryInactive) : "not_supported") << ",";
	file << (data.pageInRate != -213769 ? std::to_string(data.pageInRate) : "not_supported") << ",";
	file << (data.pageOutRate != -213769 ? std::to_string(data.pageOutRate) : "not_supported") << ",";
	file << (data.pageFaultRate != -213769 ? std::to_string(data.pageFaultRate) : "not_supported") << ",";
	file << (data.pageFaultsMajorRate != -213769 ? std::to_string(data.pageFaultsMajorRate) : "not_supported") << ",";
	file << (data.pageFreeRate != -213769 ? std::to_string(data.pageFreeRate) : "not_supported") << ",";
	file << (data.pageActivateRate != -213769 ? std::to_string(data.pageActivateRate) : "not_supported") << ",";
	file << (data.pageDeactivateRate != -213769 ? std::to_string(data.pageDeactivateRate) : "not_supported") << ",";
	file << (data.memoryReadRate != -213769 ? std::to_string(data.memoryReadRate) : "not_supported") << ",";
	file << (data.memoryWriteRate != -213769 ? std::to_string(data.memoryWriteRate) : "not_supported") << ",";
	file << (data.memoryIoRate != -213769 ? std::to_string(data.memoryIoRate) : "not_supported") << ",";
	file << (data.memoryPower != -213769 ? std::to_string(data.memoryPower) : "not_supported") << ",";

}
void writeToFileNetworkMetrics(std::ofstream &file, NetworkMetrics data) {
	file << ((data.receiveRate == -213769) ? "not_supported," : (std::to_string(data.receiveRate) + ","));
	file << ((data.receivePacketRate == -213769) ? "not_supported," : (std::to_string(data.receivePacketRate) + ","));
	file << ((data.sendRate == -213769) ? "not_supported," : (std::to_string(data.sendRate) + ","));
	file << ((data.sendPacketsRate == -213769) ? "not_supported," : (std::to_string(data.sendPacketsRate) + ","));
}

void writeToCSV(int timestamp, SystemMetrics systemMetrics, ProcessorMetrics processorMetrics, InputOutputMetrics inputOutputMetrics, NetworkMetrics networkMetrics){
    std::ofstream file("test.csv", std::ios::trunc);
    if (file.is_open()) {
		file << timestamp << ",";
        writeToFileSystemMetrics(file, systemMetrics);
		writeToFileProcessorMetrics(file, processorMetrics);
		writeToFileInputOutputMetrics(file, inputOutputMetrics);
		writeToFileNetworkMetrics(file, networkMetrics);
		file << std::endl;
        file.close();
    } else {
        std::cerr << "Unable to open file " << "test.csv" << " for writing" << std::endl;
    }
}

int main() {
	SystemMetrics systemMetrics;
	ProcessorMetrics processorMetrics;
	InputOutputMetrics inputOutputMetrics;
	NetworkMetrics networkMetrics;

	getSystemMetrics(systemMetrics);
	getProcessorMetrics(processorMetrics);
	getInputOutputMetrics(inputOutputMetrics);
	getNetworkMetrics(networkMetrics);

	writeToCSV(1, systemMetrics, processorMetrics, inputOutputMetrics, networkMetrics);

	// DEBUG
	std::cout << "\n Interrupt Rate = " << systemMetrics.interruptRate 
		<< " Context switch rate = " << systemMetrics.contextSwitchRate << " "
		<< " All processes = " << systemMetrics.processesAll << " "
		<< " Running processes = " << systemMetrics.processesRunning << " "
		<< " Blocked processes = " << systemMetrics.processesBlocked;
	return 0;
}