//
//	metrics.h - header file with functions related to defining and gathering the metrics
//
//	2022-2023	Damian Strojek @damianStrojek
// 			Piotr Garbowski @dideek
// 			Jakub Wasniewski @wisnia01
//

// External libraries
#include <string>

#ifndef METRICS_H
#define METRICS_H
#define GPROCESSID 1				// PID of process that we are focused on (G stands for global)

struct SystemMetrics {
	int processesRunning;			// Number of processes in the R state
	int processesAll;			// Number of all processes
	int processesBlocked;			// Number of processes waiting for I/O operation to complete
	int contextSwitchRate;			// Number of context switches per second
	int interruptRate;			// Number of all interrupts handled per second

	SystemMetrics();
    	void printSystemMetrics();
};

struct ProcessorMetrics {
	int timeUser;				// Time spent in user space
	int timeNice;				// Time spent in user with low priority space
	int timeSystem;				// Time spent in system space
	int timeIdle;				// Time spent on idle task
	int timeIoWait;				// Time spent waiting for I/O operation to complete
	int timeIRQ;				// Interrupt handling time
	int timeSoftIRQ;			// SoftIRQ handling time
	int timeSteal;				// Time spent in other OSs in visualization mode
	int timeGuest;				// Virtual CPU uptime for other OSs under kernel control
	int instructionsRetired;		// Number of instructions executed by the processor
	int cycles;				// Number of cycles executed by the processor
	float frequencyRelative;		// CPU clock frequency in MHz
	float unhaltedFrequency;		// unhalted CPU clock frequency in MHz
	int cacheL2Requests;			// L2 cache requests issued by the processor
	int cacheL2Misses;			// L2 cache misses
	int cacheLLCLoads;			// Number of cache loads from the Last Level Cache
	int cacheLLCStores;			// Number of cache stores to the LLC
	int cacheLLCLoadMisses;			// Number of LLC load misses 
	float cacheLLCLoadMissRate;		// LLC load misses divided by LLC loads
	int cacheLLCStoreMisses;		// Number of LLC store misses
	float cacheLLCStoreMissRate;		// LLC store misses divided by LLC stores

    	ProcessorMetrics();
    	void printProcessorMetrics();
};

struct InputOutputMetrics {
	int processID;				// Process ID of a given task 
	float dataRead;				// Data read
	float readTime;				// Data read time
	int readOperationsRate;			// Amount of read operations per second
	float dataWritten;			// Data written
	float writeTime;			// Data write time
	int writeOperationsRate;		// Amount of write operations per second
	float flushTime;			// Flush execution time
	float flushOperationsRate;		// Amount of flush operations per second

	InputOutputMetrics();
   	void printInputOutputMetrics();
};

struct MemoryMetrics {
	float memoryUsed;			// RAM used
	float memoryCached;			// Cache for files read from disk
	float swapUsed;				// Swap memory used
	float swapCached;			// Data previously written from memory to disk,
						// fetched back and still in the swap file
	float memoryActive;			// Data used in the last period
	float memoryInactive;			// Data used before memoryActive
	float pageInRate;			// Pages read
	float pageOutRate;			// Pages saved
	float pageFaultRate;			// No page status
	float pageFaultsMajorRate;		// Page missing (need to load from disk)
	float pageFreeRate;			// Page release
	float pageActivateRate;			// Page activation
	float pageDeactivateRate;		// Page deactivation
	float memoryReadRate;			// Reading from memory
	float memoryWriteRate;			// Writing to memory
	float memoryIoRate;			// Requests to read/write data from all I/O devices

	MemoryMetrics();
    	void printMemoryMetrics();
};

struct NetworkMetrics {
	int receivedData;			// All of the packets sent
	float receivePacketRate;		// packets that are being received in KB/s
	int sentData;				// All of the packets sent
	float sendPacketsRate;			// packets that are being sent in KB/s

	NetworkMetrics();
    	void printNetworkMetrics();
};

struct PowerMetrics {
	float processorPower;			// Power consumed by processor
	float memoryPower;			// Power consumed by memory
	float systemPower;			// Power consumed by system overall
	float gpuPower;		// Power consumed by GPU
	unsigned long long gpuPowerHours;	// Power consumed by GPU in Wh
	float gpuTemperature;
	float gpuFanSpeed;
	float gpuMemoryTotal;
	float gpuMemoryUsed;
	float gpuMemoryFree;
	float gpuClocksCurrentSM;
	float gpuClocksCurrentMemory;

	PowerMetrics();
	void printPowerMetrics();
};

struct AllMetrics {
	SystemMetrics systemMetrics;
	ProcessorMetrics processorMetrics;
	InputOutputMetrics inputOutputMetrics;
	MemoryMetrics memoryMetrics;
	NetworkMetrics networkMetrics;
	PowerMetrics powerMetrics;

	AllMetrics();
};

// Fetching the metrics into structures
void getSystemMetrics(SystemMetrics&);
void getProcessorMetrics(ProcessorMetrics&);
void getInputOutputMetrics(InputOutputMetrics&);
void getMemoryMetrics(MemoryMetrics&);
void getNetworkMetrics(NetworkMetrics&);
void getPowerMetrics(PowerMetrics&, bool&, bool&);
// Getting the output from system to string
std::string exec(const char*);

#endif