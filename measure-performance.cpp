// Gdansk University of Technology 2023
// Group project nr 8 realised at WETI KASK
// Description: An application for monitoring performance and energy consumption in a computing cluster
// Developers: Damian Strojek, Piotr Garbowski, Jakub Wasniewski
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
//#include <rapl.h>
//#include <nvml.h>

#define NOTSUPPORTED -666				// Functionality not yet supported
#define GPROCESSID 1					// PID of process that we are focused on (G stands for global)

struct SystemMetrics {
	int processesRunning;			// Number of processes in the R state
	int processesAll;				// Number of all processes
	int processesBlocked;			// Number of processes waiting for I/O operation to complete
	int contextSwitchRate;			// Number of context switches per second
	int interruptRate;				// Number of all interrupts handled per second

	SystemMetrics(){
		this->processesRunning = -1;
		this->processesAll = -1;
		this->processesBlocked = -1;
		this->contextSwitchRate = -1;
		this->interruptRate = -1;
	};

	void printSystemMetrics(){
   		std::cout << "\n\t[SYSTEM METRICS]\n\n"
			<< "Interrupt Rate = " << this->interruptRate << " interrupts/sec\n"
			<< "Context Switch Rate = " << this->contextSwitchRate << " switches/sec\n"
			<< "All Processes = " << this->processesAll << "\n"
			<< "Running Processes = " << this->processesRunning << "\n"
			<< "Blocked Processes = " << this->processesBlocked << "\n";
	};
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
	int instructionsRetired;		// Number of instructions executed by the processor
	int cycles;						// Number of cycles executed by the processor
	float frequencyRelative;		// CPU clock frequency in MHz
	float unhaltedFrequency;		// unhalted CPU clock frequency in MHz
	float cacheL2HitRate;			// L2 cache hits for demand data reads / all demand
									// data reads to L2 cache
	float cacheL2MissRate;			// L2 cache misses for demand data reads / all demand 
									// data reads to L2 cache
	float cacheL3HitRate;			// LLC cache hits / (LLC cache hits + LLC cache misses)
	float cacheL3HitSnoopRate;		// LLC cache misses / (LLC cache hits + LLC cache misses)
	float cacheL3MissRate;			// LLC cache hits / (LLC cache hits + snoop stalls on 
									// the bus due to LLC reference requests)

	ProcessorMetrics(){
		this->timeUser = -1;
		this->timeNice = -1;
		this->timeSystem = -1;
		this->timeIdle = -1;
		this->timeIoWait = -1;
		this->timeIRQ = -1;
		this->timeSoftIRQ = -1;
		this->timeSteal = -1;
		this->timeGuest = -1;
		this->instructionsRetired = -1;
		this->cycles = -1;
		this->frequencyRelative = -1;
		this->unhaltedFrequency = -1;
		this->cacheL2HitRate = -1;
		this->cacheL2MissRate = -1;
		this->cacheL3HitRate = -1;
		this->cacheL3HitSnoopRate = -1;
		this->cacheL3MissRate = -1;
	};

	void printProcessorMetrics(){
    	std::cout << "\n\t[PROCESSOR METRICS]\n\n"
			<< "Time User = " << this->timeUser  << "\n"
			<< "Time Nice = " << this->timeNice << "\n"
			<< "Time System = " << this->timeSystem << "\n"
			<< "Time Idle = " << this->timeIdle << "\n"
			<< "Time I/O Wait = " << this->timeIoWait << "\n"
			<< "Time IRQ = " << this->timeIRQ << "\n"
			<< "Time Soft IRQ = " << this->timeSoftIRQ << "\n"
			<< "Time Steal = " << this->timeSteal << "\n"
			<< "Time Guest = " << this->timeGuest << "\n"
			<< "Retired Instructions = " << this->instructionsRetired << "\n"
			<< "Cycles = " << this->cycles << "\n"
			<< "Relative Frequency = " << this->frequencyRelative << "\n"
			<< "Unhalted Frequency = " << this->unhaltedFrequency << "\n"
			<< "Cache L2 Hit Rate = " << this->cacheL2HitRate << "\n"
			<< "Cache L2 Miss Rate = " << this->cacheL2MissRate << "\n"
			<< "Cache L3 Hit Rate = " << this->cacheL3HitRate << "\n"
			<< "Cache L3 Miss Rate = " << this->cacheL3MissRate << "\n"
			<< "Cache L3 Hit Snoop Rate = " << this->cacheL3HitSnoopRate << "\n"; 
	};
};

struct InputOutputMetrics {
	int processID;					// Process ID of a given task 
	float dataRead;					// Data read
	float readTime;					// Data read time
	int readOperationsRate;			// Amount of read operations per second
	float dataWritten;				// Data written
	float writeTime;				// Data write time
	int writeOperationsRate;		// Amount of write operations per second
	float flushTime;				// Flush execution time
	float flushOperationsRate;		// Amount of flush operations per second

	InputOutputMetrics(){
		this->processID = GPROCESSID;
		this->dataRead = -1;
		this->readTime = -1;
		this->readOperationsRate = -1;
		this->dataWritten = -1;
		this->writeTime = -1;
		this->writeOperationsRate = -1;
		this->flushTime = -1;
		this->flushOperationsRate = -1;
	};

	void printInputOutputMetrics(){
    	std::cout << "\n\t[INPUT/OUTPUT METRICS]\n\n"
			<< "Process ID = " << this->processID << "\n"
			<< "Data Read = " << this->dataRead << " MB\n"
			<< "Read Time = " << this->readTime << " ms\n"
			<< "Read Operations Rate = " << this->readOperationsRate << "\n"
			<< "Data Written = " << this->dataWritten << " MB\n"
			<< "Write Time = " << this->writeTime << " ms\n"
			<< "Write Operations Rate = " << this->writeOperationsRate << "\n"
			<< "Flush Time = " << this->flushTime << " ms\n"
			<< "Flush Operations Rate = " << this->flushOperationsRate << "\n";
	};
};

struct MemoryMetrics {
	float memoryUsed;				// RAM used
	float memoryCached;				// Cache for files read from disk
	float swapUsed;					// Swap memory used
	float swapCached;				// Data previously written from memory to disk,
									// fetched back and still in the swap file
	float memoryActive;				// Data used in the last period
	float memoryInactive;			// Data used before memoryActive
	float pageInRate;				// Pages read
	float pageOutRate;				// Pages saved
	float pageFaultRate;			// No page status
	float pageFaultsMajorRate;		// Page missing (need to load from disk)
	float pageFreeRate;				// Page release
	float pageActivateRate;			// Page activation
	float pageDeactivateRate;		// Page deactivation
	float memoryReadRate;			// Reading from memory
	float memoryWriteRate;			// Writing to memory
	float memoryIoRate;				// Requests to read/write data from all I/O devices

	MemoryMetrics(){
		this->memoryUsed = -1;
		this->memoryCached = -1;
		this->swapUsed = -1;
		this->swapCached = -1;
		this->memoryActive = -1;
		this->memoryInactive = -1;
		this->pageInRate = -1;
		this->pageOutRate = -1;
		this->pageFaultRate = -1;
		this->pageFaultsMajorRate = -1;
		this->pageFreeRate = -1;
		this->pageActivateRate = -1;
		this->pageDeactivateRate = -1;
		this->memoryReadRate = -1;
		this->memoryWriteRate = -1;
		this->memoryIoRate = -1;
	};

	void printMemoryMetrics(){
    std::cout << "\n\t[MEMORY METRICS]\n\n"
              << "Memory Used = " << this->memoryUsed << " MB\n"
              << "Memory Cached = " << this->memoryCached << " MB\n"
              << "Swap Used = " << this->swapUsed << " MB\n"
              << "Swap Cached = " << this->swapCached << " MB\n"
              << "Memory Active = " << this->memoryActive << " MB\n"
              << "Memory Inactive = " << this->memoryInactive << " MB\n"
              << "Page Read Rate = " << this->pageInRate << "\n"
              << "Page Save Rate = " << this->pageOutRate << "\n"
              << "Page Fault Rate = " << this->pageFaultRate << "\n"
              << "Page Fault Major Rate = " << this->pageFaultsMajorRate << "\n"
              << "Page Release Rate = " << this->pageFreeRate << "\n"
              << "Page Activate Rate = " << this->pageActivateRate << "\n"
              << "Page Deactivate Rate = " << this->pageDeactivateRate << "\n"
              << "Memory Read Rate = " << this->memoryReadRate << " MB/s\n"
              << "Memory Write Rate = " << this->memoryWriteRate << " MB/s\n"
              << "Memory I/O Rate = " << this->memoryIoRate << " MB/s\n";
	};
};

struct NetworkMetrics {
	int receivedData;				// All of the packets sent
	float receivePacketRate;		// packets that are being received in KB/s
	int sentData;					// All of the packets sent
	float sendPacketsRate;			// packets that are being sent in KB/s

	NetworkMetrics(){
		this->receivedData = -1;
		this->receivePacketRate = -1;
		this->sentData = -1;
		this->sendPacketsRate = -1;
	};

	void printNetworkMetrics(){
    	std::cout << "\n\t[NETWORK METRICS]\n\n"
			<< "Receive Packet Rate = " << this->receivePacketRate << " KB/s\n"
			<< "Send Packet Rate = " << this->sendPacketsRate << " KB/s\n"
			<< "Packets Received = " << this->receivedData << "\n"
			<< "Packets Sent = " << this->sentData << "\n";
	};
};

struct PowerMetrics {
	float coresPower;					// Power consumed by cores
	float processorPower;				// Power consumed by processor
	float memoryPower;					// Power consumed by memory
	float systemPower;					// Power consumed by system overall
	unsigned long long gpuPower;		// Power consumed by GPU
	unsigned long long gpuPowerHours;	// Power consumed by GPU in Wh

	PowerMetrics(){
		this->coresPower = -1;
		this->processorPower = -1;
		this->memoryPower = -1;
		this->systemPower = -1;
		this->gpuPower = 1;
		this->gpuPowerHours = 1;
	};

	void printPowerMetrics(){
    	std::cout << "\n\t[POWER METRICS]\n"
			<< "\nCores Power = " << this->coresPower << "W"
			<< "\nProcessor = " << this->processorPower << "W"
			<< "\nMemory = " << this->memoryPower << "W"
			<< "\nSystem = " << this->systemPower << "W"
			<< "\nGPU = " << this->gpuPower << "W"
			<< "\nGPU = " << this->gpuPowerHours << "Wh\n";
	};
};

struct MetricsStruct{
	SystemMetrics systemMetrics;
	ProcessorMetrics processorMetrics;
	InputOutputMetrics inputOutputMetrics;
	MemoryMetrics memoryMetrics;
	NetworkMetrics networkMetrics;
	PowerMetrics powerMetrics;
};

// Fetching the metrics into structures
void getSystemMetrics(SystemMetrics&);
void getProcessorMetrics(ProcessorMetrics&);
void getInputOutputMetrics(InputOutputMetrics&);
void getMemoryMetrics(MemoryMetrics&);
void getNetworkMetrics(NetworkMetrics&);
void getPowerMetrics(PowerMetrics&, bool&, bool&);

// Write to file functions
void writeToFileSystemMetrics(std::ofstream&, SystemMetrics);
void writeToFileProcessorMetrics(std::ofstream&, ProcessorMetrics);
void writeToFileInputOutputMetrics(std::ofstream&, InputOutputMetrics);
void writeToFileMemoryMetrics(std::ofstream&, MemoryMetrics);
void writeToFileNetworkMetrics(std::ofstream&, NetworkMetrics);
void writeToCSV(std::ofstream&, std::string, SystemMetrics, ProcessorMetrics, 
					InputOutputMetrics, MemoryMetrics, NetworkMetrics);

// Printing for the user
void printMetric(std::string, int, std::string);
void printMetricPair(std::string, int, std::string, std::string, int, std::string);
void printMetrics(SystemMetrics*, ProcessorMetrics*, InputOutputMetrics*, 
					MemoryMetrics*, NetworkMetrics*);

int keyboardHit(void);
std::string exec(const char*);

int main(){

	SystemMetrics systemMetrics;
	ProcessorMetrics processorMetrics;
	InputOutputMetrics inputOutputMetrics;
	MemoryMetrics memoryMetrics;
	NetworkMetrics networkMetrics;
	PowerMetrics powerMetrics;

	const char* dateCommand = "date +'%d%m%y-%H%M%S'",
		*processCommand = "ps -p 1 > /dev/null && echo '1' || echo '0'";	// [TODO] Add GPROCESSID
	std::string timestamp = exec(dateCommand);
	timestamp.pop_back();
	/*std::string fileName = timestamp += "_metrics.csv";
	std::ofstream file(fileName, std::ios::out);
	if(!file.is_open()) std::cerr << "\n\n\t [ERROR] Unable to open file " << fileName << " for writing.\n";*/

	// If there is any error with the RAPL and NVML libraries
	// we are not trying to measure processor and GPU power
	bool raplError = 0, nvmlError = 0;
	/*if(rapl_init()){
		raplError = 1;
		std::cout << "\n\n\t[WARNING] RAPL Library not initialized\n";
	}
	if(nvmlInit() != NVML_SUCCESS){
		nvmlError = 1;
		std::cout << "\n\n\t[WARNING] NVML Library not initialized\n";
	}*/

	// Checking if process with GPROCESSID is still running
	while(std::stoi(exec(processCommand))){
    	if(keyboardHit()){
			std::cout << "\n\n\t[STOP] Key pressed.\n\n";
			break;
		}	
		
		timestamp = exec(dateCommand);
		timestamp.pop_back();

		//auto start = std::chrono::high_resolution_clock::now();

		std::cout << "\n\n   [TIMESTAMP] " << timestamp << "\n";

		getSystemMetrics(systemMetrics);
		getProcessorMetrics(processorMetrics);
		getInputOutputMetrics(inputOutputMetrics);
		getMemoryMetrics(memoryMetrics);
		getNetworkMetrics(networkMetrics);
		getPowerMetrics(powerMetrics, raplError, nvmlError);

		sleep(2);

		//auto end = std::chrono::high_resolution_clock::now();
		//auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end-start);
		//std::cout << "Time taken to get all measures:" << duration.count() << "microseconds\n";

		// Display metrics
		//printMetrics(&systemMetrics, &processorMetrics, &inputOutputMetrics, &memoryMetrics, &networkMetrics);

		// Save metrics to file
	  	//writeToCSV(file, timestamp, systemMetrics, processorMetrics, inputOutputMetrics, memoryMetrics, networkMetrics);
  	}

    //file.close();
	/*if(!raplError) rapl_finish();
	if(!nvmlError) nvmlShutdown();*/
	return 0;
};

// Execute a Linux command and return the output using std::string
std::string exec(const char* cmd){
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe)
        throw std::runtime_error("popen() failed!");
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
        result += buffer.data();

	if(!result.length())
		std::cout << "\n\n\t[ERROR] String returned by exec() has length 0\n";

    return result;
};

void getSystemMetrics(SystemMetrics &systemMetrics){

	const char* command = "vmstat";
	std::string output = exec(command), temp;

	temp = output.substr(219, 4);
	systemMetrics.interruptRate = std::stoi(temp);		// interrupts/sec
	temp = output.substr(224, 4);
	systemMetrics.contextSwitchRate = std::stoi(temp);	// context switches/sec
	
	command = "cat /proc/loadavg | cut -d ' ' -f 4";
	output = exec(command);

	size_t slashPosition = output.find('/');
	temp = output.substr(0, slashPosition);
	systemMetrics.processesRunning = std::stoi(temp);	// number of processes
	temp = output.substr(slashPosition+1);
	systemMetrics.processesAll = std::stoi(temp);		// number of processes
 
	command = "ps -eo state | grep -c '^D'";
	output = exec(command);
	systemMetrics.processesBlocked = std::stoi(output);	// number of processes

	systemMetrics.printSystemMetrics();
};

void getProcessorMetrics(ProcessorMetrics &processorMetrics){

	const char* command = "cat /proc/cpuinfo | grep 'processor' -c";
	std::string output = exec(command), temp;
	int numberOfProcessors = std::stoi(output);

	command = "cat /proc/stat";
	output = exec(command);
	std::stringstream stream(output);

	stream >> temp;		// Get rid of 'cpu' at the beggining
	stream >> temp;
	processorMetrics.timeUser = std::stoi(temp);		// USER_HZ
	stream >> temp;
	processorMetrics.timeNice = std::stoi(temp);		// USER_HZ
	stream >> temp;
	processorMetrics.timeSystem = std::stoi(temp);		// USER_HZ
	stream >> temp;
	processorMetrics.timeIdle = std::stoi(temp);		// USER_HZ
	stream >> temp;
	processorMetrics.timeIoWait = std::stoi(temp);		// USER_HZ
	stream >> temp;
	processorMetrics.timeIRQ = std::stoi(temp);			// USER_HZ
	stream >> temp;
	processorMetrics.timeSoftIRQ = std::stoi(temp);		// USER_HZ
	stream >> temp;
	processorMetrics.timeSteal = std::stoi(temp);		// USER_HZ
	stream >> temp;
	processorMetrics.timeGuest = std::stoi(temp);		// USER_HZ

	// Created string and parsed in to const char* because it was too long
	std::string commandString = 
		"perf stat -e cpu/event=0x24,umask=0x01,name=L2_RQSTS_DEMAND_DATA_RD_HIT/,cpu/"
		"event=0x24,umask=0x02,name=L2_RQSTS_ALL_DEMAND_DATA_RD/,cpu/event=0x24,umask=0x04,n"
		"ame=L2_RQSTS_DEMAND_DATA_RD_MISS/,cpu/event=0x2e,umask=0x01,name=LLC_REFERENCES_LLC"
		"_HIT/,cpu/event=0x2e,umask=0x02,name=LLC_REFERENCES_LLC_MISS/,cpu/event=0x2e,umask="
		"0x08,name=LLC_REFERENCES_SNOOP_STALL/ --all-cpus sleep 1 2>&1 | awk '/L2_RQSTS_ALL_"
		"DEMAND_DATA_RD|L2_RQSTS_DEMAND_DATA_RD_HIT|L2_RQSTS_DEMAND_DATA_RD_MISS|LLC_REFEREN"
		"CES_LLC_HIT|LLC_REFERENCES_LLC_MISS|LLC_REFERENCES_SNOOP_STALL/ {print $1}'";
	command = commandString.c_str();
	output = exec(command);
	std::stringstream streamTwo(output);
	float L2RqstsHit, L2RqstsMiss, L2RqstsData, LLCHit, LLCMiss, LLCSnoop;

	streamTwo >> temp;
	L2RqstsHit = std::stof(temp);
	streamTwo >> temp;
	L2RqstsMiss = std::stof(temp);
	streamTwo >> temp;
	L2RqstsData = std::stof(temp);
	streamTwo >> temp;
	LLCHit = std::stof(temp);
	streamTwo >> temp;
	LLCMiss = std::stof(temp);
	streamTwo >> temp;
	LLCSnoop = std::stof(temp);

	processorMetrics.cacheL2HitRate = L2RqstsHit / L2RqstsData;
	processorMetrics.cacheL2MissRate = L2RqstsMiss / L2RqstsData;
	processorMetrics.cacheL3HitRate = LLCHit / (LLCHit + LLCMiss);
	processorMetrics.cacheL3MissRate = LLCMiss / (LLCHit + LLCMiss);
	processorMetrics.cacheL3HitSnoopRate = LLCHit / (LLCHit + LLCSnoop);

	command = "perf stat -e instructions,cycles,cpu-clock,cpu-clock:u sleep 1 2>&1 | awk '/^[ ]*[0-9]/{print $1}'";
	output = exec(command);
	std::stringstream streamThree(output);

	streamThree >> temp;
	processorMetrics.instructionsRetired = std::stoi(temp); 	// number of instructions
	streamThree >> temp;
	processorMetrics.cycles = std::stoi(temp); 					// number of cycles
	streamThree >> temp;
	processorMetrics.frequencyRelative = std::stof(temp);		// MHz
	streamThree >> temp;
	processorMetrics.unhaltedFrequency = std::stof(temp); 		// MHz

	processorMetrics.printProcessorMetrics();
};

void getInputOutputMetrics(InputOutputMetrics &inputOutputMetrics){

	const char* command = "awk '{ print $2 }' /proc/$$/io";		// [TODO] Change PID 
	std::string output = exec(command), temp;
	std::stringstream stream(output);
	
	stream >> temp;
	inputOutputMetrics.dataRead = std::stof(temp) / 1024;		// MB
	stream >> temp;
	inputOutputMetrics.dataWritten = std::stof(temp) / 1024;	// MB
	stream >> temp;
	inputOutputMetrics.readOperationsRate = std::stoi(temp);	// Number of operations
	stream >> temp;	
	inputOutputMetrics.writeOperationsRate = std::stoi(temp);	// Number of operations
	stream >> temp;

	command = "iostat -d -k | awk '/^[^ ]/ {device=$1} $1 ~ /sda/ {print 1000*$10/($4*$3), 1000*$11/($4*$3), $6/$4, $7/$6}'";
	output = exec(command);
	std::stringstream streamTwo(output);

	streamTwo >> temp;
	inputOutputMetrics.readTime = std::stof(temp);				// ms
	streamTwo >> temp;
	inputOutputMetrics.writeTime = std::stof(temp);				// ms
	streamTwo >> temp;
	inputOutputMetrics.flushOperationsRate = std::stof(temp);	// operations/sec
	streamTwo >> temp;
	inputOutputMetrics.flushTime = std::stof(temp);				// ms

	inputOutputMetrics.printInputOutputMetrics();
};

void getMemoryMetrics(MemoryMetrics &memoryMetrics){

	const char* command = "grep -v -e 'anon' -e 'file' /proc/meminfo | grep -E '^(Cached|SwapCached|SwapTotal|SwapFree|Active|Inactive)' | awk '{print $2}'";
	std::string output = exec(command), temp, swapFree;
	std::stringstream stream(output);

	stream >> temp;
	memoryMetrics.memoryCached = std::stof(temp);
	memoryMetrics.memoryCached = memoryMetrics.memoryCached / 1024;		// MB
	stream >> temp;
	memoryMetrics.swapCached = std::stof(temp);
	memoryMetrics.swapCached = memoryMetrics.swapCached / 1024;			// MB
	stream >> temp;
	memoryMetrics.memoryActive = std::stof(temp);
	memoryMetrics.memoryActive = memoryMetrics.memoryActive / 1024;		// MB
	stream >> temp;
	memoryMetrics.memoryInactive = std::stof(temp);
	memoryMetrics.memoryInactive = memoryMetrics.memoryInactive / 1024;	// MB
	stream >> temp;
	stream >> swapFree;
	memoryMetrics.swapUsed = std::stof(temp) - std::stof(swapFree);
	memoryMetrics.swapUsed = memoryMetrics.swapUsed / 1024;				// MB

	command = "sar -r -B 1 1 | awk 'NR==4{print $2,$3,$4,$5,$6,$7,$8}'";
	output = exec(command);
	std::stringstream streamTwo(output);

	streamTwo >> temp;
	memoryMetrics.pageInRate = std::stof(temp);				// pages/sec
	streamTwo >> temp;
	memoryMetrics.pageOutRate = std::stof(temp);			// pages/sec
	streamTwo >> temp;
	memoryMetrics.pageFaultRate = std::stof(temp);			// pages/sec
	streamTwo >> temp;
	memoryMetrics.pageFaultsMajorRate = std::stof(temp);	// pages/sec
	streamTwo >> temp;
	memoryMetrics.pageFreeRate = std::stof(temp);			// pages/sec
	streamTwo >> temp;	
	memoryMetrics.pageActivateRate = std::stof(temp);		// kpages/sec
	streamTwo >> temp;
	memoryMetrics.pageDeactivateRate = std::stof(temp);		// kpages/sec

	command = "sar -b 1 1 | awk 'NR==4{print $6/1024,$7/1024,($6+$7)/1024}'";
	output = exec(command);
	std::stringstream streamThree(output);

	streamThree >> temp;
	memoryMetrics.memoryReadRate = std::stof(temp);			// MB/s
	streamThree >> temp;
	memoryMetrics.memoryWriteRate = std::stof(temp);		// MB/s
	streamThree >> temp;
	memoryMetrics.memoryIoRate = std::stof(temp);			// MB/s
	
	memoryMetrics.printMemoryMetrics();
};

void getNetworkMetrics(NetworkMetrics &networkMetrics){

	const char* command = "ifstat 1 1 | tail -1 | awk '{ print $1, $2 }'";
	std::string output = exec(command), temp;
	std::stringstream stream(output);

	stream >> temp;
	networkMetrics.receivePacketRate = std::stof(temp);		// KB/sec
	stream >> temp;
	networkMetrics.sendPacketsRate = std::stof(temp);		// KB/sec

	// Default interface: eth0
	// des01 interface: ep0s31f6
	command = "cat /proc/net/dev | awk '/^ *enp0s31f6:/ {rx=$3; tx=$11; print rx,tx; exit}'";
	output = exec(command);
	std::stringstream streamTwo(output);

	streamTwo >> temp;
	networkMetrics.receivedData = std::stoi(temp);			// number of packets
	streamTwo >> temp;
	networkMetrics.sentData = std::stoi(temp);				// number of packets

	networkMetrics.printNetworkMetrics();
};

void getPowerMetrics(PowerMetrics &powerMetrics, bool& raplError, bool& nvmlError){

	const char* command = "perf stat -e power/energy-cores/,power/energy-ram/,power/energy-pkg/ sleep 1 2>&1 | awk '/Joules/ {print $1}'";
	std::string output = exec(command), temp;
	std::stringstream streamOne(output);
	
	streamOne >> temp;
	powerMetrics.coresPower = std::stof(temp);
	streamOne >> temp;
	powerMetrics.memoryPower = std::stof(temp);
	streamOne >> temp;
	powerMetrics.systemPower = std::stof(temp);
	
	/*
	if(!raplError){
		double energy;
		if (rapl_get_energy(RAPL_PACKAGE, &energy) != 0){
			std::cerr << "\n\n\t[ERROR] Failed to get package energy consumption\n";
			raplError = 1;
			rapl_finish();
		}
		else
			powerMetrics.processorPower = energy;
	}

	if(!nvmlError){
		// Get the device handle for the first GPU on the system
		nvmlDevice_t device;
		result = nvmlDeviceGetHandleByIndex(0, &device);
		if (NVML_SUCCESS != result) {
			std::cout << "\n\n\t [ERROR] Failed to get handle for GPU 0: " <<  nvmlErrorString(result) << "\n";
			nvmlError = 1;
			nvmlShutdown();
		}
		else {
			// Get the total energy consumption of the GPU in millijoules
			unsigned long long energyConsumed;
			result = nvmlDeviceGetTotalEnergyConsumption(device, &energyConsumed);
			if (NVML_SUCCESS != result) {
				std::cout << "\n\n\t [ERROR] Failed to get total energy consumption of GPU 0: " << nvmlErrorString(result) << "\n";
				nvmlError = 1;
				nvmlShutdown();
			}
			else
				powerMetrics.gpuPower = result;
		}
	}
	*/
	powerMetrics.printPowerMetrics();
};

void writeToFileSystemMetrics(std::ofstream &file, SystemMetrics data){
	file << (data.processesRunning != NOTSUPPORTED ? std::to_string(data.processesRunning) : "not_supported") << ",";
	file << (data.processesAll != NOTSUPPORTED ? std::to_string(data.processesAll) : "not_supported") << ",";
	file << (data.processesBlocked != NOTSUPPORTED ? std::to_string(data.processesBlocked) : "not_supported") << ",";
	file << (data.contextSwitchRate != NOTSUPPORTED ? std::to_string(data.contextSwitchRate) : "not_supported") << ",";
	file << (data.interruptRate != NOTSUPPORTED ? std::to_string(data.interruptRate) : "not_supported");
};

void writeToFileProcessorMetrics(std::ofstream &file, ProcessorMetrics data){
	file << (data.timeUser != NOTSUPPORTED ? std::to_string(data.timeUser) : "not_supported") << ",";
	file << (data.timeNice != NOTSUPPORTED ? std::to_string(data.timeNice) : "not_supported") << ",";
	file << (data.timeSystem != NOTSUPPORTED ? std::to_string(data.timeSystem) : "not_supported") << ",";
	file << (data.timeIdle != NOTSUPPORTED ? std::to_string(data.timeIdle) : "not_supported") << ",";
	file << (data.timeIoWait != NOTSUPPORTED ? std::to_string(data.timeIoWait) : "not_supported") << ",";
	file << (data.timeIRQ != NOTSUPPORTED ? std::to_string(data.timeIRQ) : "not_supported") << ",";
	file << (data.timeSoftIRQ != NOTSUPPORTED ? std::to_string(data.timeSoftIRQ) : "not_supported") << ",";
	file << (data.timeSteal != NOTSUPPORTED ? std::to_string(data.timeSteal) : "not_supported") << ",";
	file << (data.timeGuest != NOTSUPPORTED ? std::to_string(data.timeGuest) : "not_supported") << ",";
	file << (data.instructionsRetired != NOTSUPPORTED ? std::to_string(data.instructionsRetired) : "not_supported") << ",";
	file << (data.cycles != NOTSUPPORTED ? std::to_string(data.cycles) : "not_supported") << ",";
	file << (data.frequencyRelative != NOTSUPPORTED ? std::to_string(data.frequencyRelative) : "not_supported") << ",";
	file << (data.unhaltedFrequency != NOTSUPPORTED ? std::to_string(data.unhaltedFrequency) : "not_supported") << ",";
	file << (data.cacheL2HitRate != NOTSUPPORTED ? std::to_string(data.cacheL2HitRate) : "not_supported") << ",";
	file << (data.cacheL2MissRate != NOTSUPPORTED ? std::to_string(data.cacheL2MissRate) : "not_supported") << ",";
	file << (data.cacheL3HitRate != NOTSUPPORTED ? std::to_string(data.cacheL3HitRate) : "not_supported") << ",";
	file << (data.cacheL3HitSnoopRate != NOTSUPPORTED ? std::to_string(data.cacheL3HitSnoopRate) : "not_supported") << ",";
	file << (data.cacheL3MissRate != NOTSUPPORTED ? std::to_string(data.cacheL3MissRate) : "not_supported") << ",";
};

void writeToFileInputOutputMetrics(std::ofstream &file, InputOutputMetrics data){
	file << (data.dataRead != NOTSUPPORTED ? std::to_string(data.dataRead) : "not_supported") << ",";
	file << (data.readTime != NOTSUPPORTED ? std::to_string(data.readTime) : "not_supported") << ",";
	file << (data.readOperationsRate != NOTSUPPORTED ? std::to_string(data.readOperationsRate) : "not_supported") << ",";
	file << (data.dataWritten != NOTSUPPORTED ? std::to_string(data.dataWritten) : "not_supported") << ",";
	file << (data.writeTime != NOTSUPPORTED ? std::to_string(data.writeTime) : "not_supported") << ",";
	file << (data.writeOperationsRate != NOTSUPPORTED ? std::to_string(data.writeOperationsRate) : "not_supported") << ",";
	file << (data.flushTime != NOTSUPPORTED ? std::to_string(data.flushTime) : "not_supported") << ",";
	file << (data.flushOperationsRate != NOTSUPPORTED ? std::to_string(data.flushOperationsRate) : "not_supported") << ",";
};

void writeToFileMemoryMetrics(std::ofstream &file, MemoryMetrics data){
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
};

void writeToFileNetworkMetrics(std::ofstream &file, NetworkMetrics data){
	file << ((data.receivedData == NOTSUPPORTED) ? "not_supported," : (std::to_string(data.receivedData) + ","));
	file << ((data.receivePacketRate == NOTSUPPORTED) ? "not_supported," : (std::to_string(data.receivePacketRate) + ","));
	file << ((data.sentData == NOTSUPPORTED) ? "not_supported," : (std::to_string(data.sentData) + ","));
	file << ((data.sendPacketsRate == NOTSUPPORTED) ? "not_supported," : (std::to_string(data.sendPacketsRate) + ","));
};

void writeToCSV(std::ofstream &file, std::string timestamp, SystemMetrics systemMetrics, ProcessorMetrics processorMetrics, 
				InputOutputMetrics inputOutputMetrics, MemoryMetrics memoryMetrics, NetworkMetrics networkMetrics){

	file << timestamp << ",";
	writeToFileSystemMetrics(file, systemMetrics);
	writeToFileProcessorMetrics(file, processorMetrics);
	writeToFileMemoryMetrics(file, memoryMetrics);
	writeToFileInputOutputMetrics(file, inputOutputMetrics);
	writeToFileNetworkMetrics(file, networkMetrics);
	file << std::endl;
};

void printMetric(std::string metricName, int metricValue, std::string metricUnit){

	std::string value = std::to_string(metricValue);
	if(metricValue == NOTSUPPORTED) {
		value = "notSupported";
		metricUnit = "";
	}
	std::cout << metricName << std::right << std::setfill('.') << std::setw(30-metricName.length()) << value << metricUnit << "\n";
};

void printMetricPair(std::string metricName, int metricValue, std::string metricUnit, 
						std::string metricNameTwo, int metricValueTwo, std::string metricUnitTwo){

	std::string value = std::to_string(metricValue);
	if(metricValue == NOTSUPPORTED) {
		value = "notSupported";
		metricUnit = "";
	}

	std::string value2 = std::to_string(metricValueTwo);
	if(metricValueTwo == NOTSUPPORTED) {
		value2 = "notSupported";
		metricUnitTwo = "";
	}

	std::cout << metricName << std::right << std::setfill('.') << std::setw(30-metricName.length()) << value << metricUnit;
	for(int i=0; i<20-metricUnit.length(); i++) std::cout << ' ';

	std::cout << std::left << metricNameTwo << std::right << std::setfill('.') << 
		std::setw(30-metricNameTwo.length()) << value2 << metricUnitTwo << std::endl;
};

void printMetrics(SystemMetrics* systemMetrics, ProcessorMetrics* processorMetrics, 
					InputOutputMetrics* inputOutputMetrics, MemoryMetrics* memoryMetrics, NetworkMetrics* networkMetrics){

	auto now = std::chrono::system_clock::now();
  	std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::cout << std::put_time(std::localtime(&now_c), "%Y-%m-%d %X") << '\n';

	std::cout << "System:";
	for(int i=0;i<43;i++) std::cout << ' ';

	std::cout << "Network:" << '\n';
	printMetricPair("Processes running", systemMetrics->processesRunning,"", "Received packets",networkMetrics->receivedData,"");
	printMetricPair("All Processes", systemMetrics->processesAll,"","Received packets rate", networkMetrics->receivePacketRate,"KB/s");
	printMetricPair("Context Switch Rate", systemMetrics->contextSwitchRate,"/s","Sent packets",networkMetrics->sentData,"");
	printMetricPair("Interrupt rate", systemMetrics->interruptRate,"/s","Sent packets rate",networkMetrics->sendPacketsRate,"KB/s");
	std::cout << '\n';

	std::cout << "Memory:";
	for(int i=0;i<43;i++) std::cout << ' ';

	std::cout << "Processor:" << '\n';
	printMetricPair("Memory used", memoryMetrics->memoryUsed,"MB","Time user",processorMetrics->timeUser,"USER_HZ");
	printMetricPair("Memory cached", memoryMetrics->memoryCached,"MB","Time system",processorMetrics->timeSystem,"USER_HZ");
	printMetricPair("Swap used", memoryMetrics->swapUsed,"MB","Time idle",processorMetrics->timeIdle,"USER_HZ");
	printMetricPair("Swap cached", memoryMetrics->swapCached,"MB","Time I/O wait",processorMetrics->timeIoWait,"USER_HZ");
	printMetricPair("Memory Active", memoryMetrics->memoryActive,"MB","Time IRQ",processorMetrics->timeIRQ,"USER_HZ");
	printMetricPair("Memory Inactive", memoryMetrics->memoryInactive,"MB","Time Steal",processorMetrics->timeSteal,"USER_HZ");

	std::cout << '\n'<< "I/O for PID 1:" << '\n';
	printMetric("Data Read ",inputOutputMetrics->dataRead,"MB");
	printMetric("Data Written ",inputOutputMetrics->dataWritten,"MB");
	printMetric("Read operations ",inputOutputMetrics->readOperationsRate,"");
	printMetric("Write operations ",inputOutputMetrics->writeOperationsRate,"");
	std::cout << '\n';
};

int keyboardHit(void){
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
