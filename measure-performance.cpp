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
//#include <rapl.h>						// [TODO] Not supported yet
//#include <nvml.h>						// [TODO] Not supported yet

#define NOTSUPPORTED -666				// Functionality not yet supported
#define GPROCESSID 1					// PID of process that we are focused on (g stands for global)

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
		std::cout << "\n\t[SYSTEM METRICS]\n\nInterrupt Rate = " << this->interruptRate << " interrupts/sec\nContext Switch Rate = " 
		<< this->contextSwitchRate << " switches/sec\nAll Processes = " << this->processesAll <<  "\nRunning Processes = " << 
		this->processesRunning << "\nBlocked Processes = " << this->processesBlocked << "\n";
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
		this->instructionsRetiredRate = -1;
		this->cyclesRate = -1;
		this->cyclesReferenceRate = -1;
		this->frequencyRelative = -1;
		this->frequencyActiveRelative = -1;
		this->cacheL2HitRate = -1;
		this->cacheL2MissRate = -1;
		this->cacheL3HitRate = -1;
		this->cacheL3HitSnoopRate = -1;
		this->cacheL3MissRate = -1;
	};

	void printProcessorMetrics(){
		std::cout << "\n\t[PROCESSOR METRICS]\n\nTime User = " << this->timeUser  << "\nTime Nice = " << this->timeNice << "\nTime System = " << 
		this->timeSystem << "\nTime Idle = " << this->timeIdle << "\nTime I/O Wait = " << this->timeIoWait << "\nTime IRQ = " << this->timeIRQ << 
		"\nTime Soft IRQ = " << this->timeSoftIRQ << "\nTime Steal = " << this->timeSteal << "\nTime Guest = " << this->timeGuest << 
		"\nInstructions Retired Rate = " << this->instructionsRetiredRate << "\nCycles rate = " << this->cyclesRate << "\nCycles reference rate = " << 
		this->cyclesReferenceRate << "\nRelative Frequency = " << this->frequencyRelative << "\nActive Relative Frequency = " << 
		this->frequencyActiveRelative << "\nCache L2 Hit Rate = " << this->cacheL2HitRate << "\nCache L2 Miss Rate = " << this->cacheL2MissRate << 
		"\nCache L3 Hit Rate = " << this->cacheL3HitRate << "\nCache L3 Miss Rate = " << this->cacheL3MissRate << "\nCache L3 Hit Snoop Rate = " << 
		this->cacheL3HitSnoopRate << "\n"; 
	};
};

struct InputOutputMetrics {
	int processID;					// Process ID of a given task 
	int dataRead;					// Data read
	int readTime;					// Data read time
	int readOperationsRate;			// Amount of read operations per second
	int dataWritten;				// Data written
	int writeTime;					// Data write time
	int writeOperationsRate;		// Amount of write operations per second
	int flushTime;					// Flush execution time
	int flushOperationsRate;		// Amount of flush operations per second

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

	void printInputOuputMetrics(){
		std::cout << "\n\t[INPUT/OUTPUT METRICS]\n\nProcess ID = " << this->processID << "\nData Read = " << this->dataRead << 
		" MB\nRead Time = " << this->readTime << " ms\nRead Operations Rate = " << this->readOperationsRate << " operations [TO CHANGE]\nData Written = " 
		<< this->dataWritten << " MB\nWrite Time = " << this->writeTime << " ms\nWrite Operations Rate = " << this->writeOperationsRate << 
		" operations [TO CHANGE]\nFlush Time = " << this->flushTime << " ms\nFlush Operations Rate = " << this->flushOperationsRate << " ops/sec\n";
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
	int pageInRate;					// Pages read
	int pageOutRate;				// Pages saved
	int pageFaultRate;				// No page status
	int pageFaultsMajorRate;		// Page missing (need to load from disk)
	int pageFreeRate;				// Page release
	int pageActivateRate;			// Page activation
	int pageDeactivateRate;			// Page deactivation
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
		std::cout << "\n\t[MEMORY METRICS]\n\nMemory Used = " << this->memoryUsed << " MB\nMemory Cached = " << this->memoryCached << 
		" MB\nSwap Used = " << this->swapUsed << " MB\nSwap Cached = " << this->swapCached << " MB\nMemory Active = " << this->memoryActive << 
		" MB\nMemory Inactive = " << this->memoryInactive << " MB\nPage Read Rate = " << this->pageInRate << "\nPage Save Rate = " << 
		this->pageOutRate << "\nPage Fault Rate = " << this->pageFaultRate << "\nPage Fault Major Rate = " << this->pageFaultsMajorRate << 
		"\nPage Release Rate = " << this->pageFreeRate << "\nPage Activate Rate = " << this->pageActivateRate << "\nPage Deactivate Rate = " << 
		this->pageDeactivateRate << "\nMemore Read Rate = " << this->memoryReadRate << " MB/s\nMemory Write Rate = " << this->memoryWriteRate << 
		" MB/s\nMemory I/O Rate = " << this->memoryIoRate << " MB/s\n";  
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
		std::cout << "\n\t[NETWORK METRICS]\n\nReceive Packet Rate = " << this->receivePacketRate << " KB/s\nSend Packet Rate = " << 
		this->sendPacketsRate << " KB/s\nPackets Received = " << this->receivedData << "\nPackets Sent = " << this->sentData << "\n";
	};
};

struct PowerMetrics {
	int processorPower;					// Power consumed by processor
	int memoryPower;					// Power consumed by memory
	unsigned long long gpuPower;		// Power consumed by GPU
	unsigned long long gpuPowerHours;	// Power consumed by GPU in Wh

	PowerMetrics(){
		this->processorPower = -1;
		this->memoryPower = -1;
		this->gpuPower = 1;
		this->gpuPowerHours = 1;
	};

	void printPowerMetrics(){
		std::cout << "\n\t[POWER METRICS]\n\nProcessor = " << this->processorPower << "W\nMemory = " << this->memoryPower << 
		"W\nGPU = " << this->gpuPower << "W\nGPU = " << this->gpuPowerHours << "Wh\n";
	};
};

std::string exec(const char* cmd);
void getSystemMetrics(SystemMetrics &systemMetrics);
void getProcessorMetrics(ProcessorMetrics &processorMetrics);
void getInputOutputMetrics(InputOutputMetrics &inputOutputMetrics);
void getMemoryMetrics(MemoryMetrics &memoryMetrics);
void getNetworkMetrics(NetworkMetrics &networkMetrics);
void getPowerMetrics(PowerMetrics &powerMetrics);
void writeToFileSystemMetrics(std::ofstream &file, SystemMetrics data);
void writeToFileProcessorMetrics(std::ofstream &file, ProcessorMetrics data);
void writeToFileInputOutputMetrics(std::ofstream &file, InputOutputMetrics data);
void writeToFileMemoryMetrics(std::ofstream &file, MemoryMetrics data);
void writeToFileNetworkMetrics(std::ofstream &file, NetworkMetrics data);
void writeToCSV(std::ofstream &file, std::string timestamp, SystemMetrics systemMetrics, ProcessorMetrics processorMetrics, 
						InputOutputMetrics inputOutputMetrics, MemoryMetrics memoryMetrics, NetworkMetrics networkMetrics);
void printMetric(std::string metricName, int metricValue, std::string metricUnit);
void printMetricPair(std::string metricName, int metricValue, std::string metricUnit, 
						std::string metricNameTwo, int metricValueTwo, std::string metricUnitTwo);
void printMetrics(SystemMetrics* systemMetrics, ProcessorMetrics* processorMetrics, 
						InputOutputMetrics* inputOutputMetrics, MemoryMetrics* memoryMetrics, NetworkMetrics* networkMetrics);
int keyboardHit(void);

int main(){

	SystemMetrics systemMetrics;
	ProcessorMetrics processorMetrics;
	InputOutputMetrics inputOutputMetrics;
	MemoryMetrics memoryMetrics;
	NetworkMetrics networkMetrics;
	PowerMetrics powerMetrics;

	const char* dateCommand = "date +'%d%m%y-%H%M%S'",
		*processCommand = "ps -p 1 > /dev/null && echo '1' || echo '0'";
	std::string timestamp = exec(dateCommand);
	timestamp.pop_back();
	/*std::string fileName = timestamp += "_metrics.csv";
	std::ofstream file(fileName, std::ios::out);
	if(!file.is_open()) std::cerr << "\n\n\t [ERROR] Unable to open file " << fileName << " for writing.\n";*/

	// Checking if process with PROCESSID is still running
	while(std::stoi(exec(processCommand))){
    	if(keyboardHit()){
			std::cout << "\n\n\t [STOP] Key pressed.\n\n";
			break;
		}	
		
		timestamp = exec(dateCommand);
		timestamp.pop_back();

		//auto start = std::chrono::high_resolution_clock::now();

		std::cout << "\n\n  [TIMESTAMP] " << timestamp << "\n";

		getSystemMetrics(systemMetrics);
		getProcessorMetrics(processorMetrics);
		getInputOutputMetrics(inputOutputMetrics);
		getMemoryMetrics(memoryMetrics);
		getNetworkMetrics(networkMetrics);
		getPowerMetrics(powerMetrics);

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
		std::cout << "\n\n\t [ERROR] String returned by exec() has length 0\n";

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

	/*
	// Not supported metrics:
	processorMetrics.cacheL2HitRate = NOTSUPPORTED;
	processorMetrics.cacheL2MissRate = NOTSUPPORTED;
	processorMetrics.cacheL3HitRate = NOTSUPPORTED;
	processorMetrics.cacheL3MissRate = NOTSUPPORTED;
	processorMetrics.cacheL3HitSnoopRate = NOTSUPPORTED;
	processorMetrics.instructionsRetiredRate = NOTSUPPORTED;
	processorMetrics.cyclesRate = NOTSUPPORTED;
	processorMetrics.cyclesReferenceRate = NOTSUPPORTED;
	processorMetrics.frequencyRelative = NOTSUPPORTED;
	processorMetrics.frequencyActiveRelative = NOTSUPPORTED;

	// [TODO] as far as we know perf works only on hardware and we can't test it yet
	// sudo perf stat -e LLC-loads,LLC-load-misses,L2_RQSTS.ALL,L2_RQSTS.MISS,PERF_COUNT_HW_CPU_CYCLES,PERF_COUNT_HW_INSTRUCTIONS,PERF_COUNT_HW_REF_CPU_CYCLES,PERF_COUNT_HW_CPU_CYCLES:REF_XCLK,PERF_COUNT_HW_CPU_CYCLES:UNHALTED_CORE_CYCLES sleep 1 2>&1 | awk '/LLC-loads/ { ll=$1 } /LLC-load-misses/ { lm=$1 } /L2_RQSTS.ALL/ { l2=$1 } /L2_RQSTS.MISS/ { lm2=$1 } /CPU_CYCLES:/ { cpu=$1 } /INSTRUCTIONS/ { instr=$1 } /REF_CPU_CYCLES/ { ref=$1 } /REF_XCLK/ { xclk=$1 } /UNHALTED_CORE_CYCLES/ { unhalted=$1 } END { printf "L2 Cache Hit Rate: %f%%\n", (l2-lm2)*100/l2; printf "L2 Cache Miss Rate: %f%%\n", lm2*100/l2; printf "L3 Cache Hit Rate: %f%%\n", (ll-lm)*100/ll; printf "L3 Cache Miss Rate: %f%%\n", lm*100/ll; printf "Instructions Retired Rate: %f instructions/cycle\n", instr/cpu; printf "Processor Cycle Metrics: %f cycles/instruction\n", cpu/instr; printf "Processor Cycles Reference Rate: %f cycles/second\n", cpu/ref; printf "Relative Frequency: %f GHz\n", xclk/unhalted/1e9; printf "Active Relative Frequency: %f GHz\n", xclk/cpu/1e9 }'
	command = "sudo perf stat -e LLC-loads,LLC-load-misses,L2_RQSTS.ALL,L2_RQSTS.MISS,PERF_COUNT_HW_CACHE_L3_HITS sleep 1 2>&1 |" + 
	" awk '/LLC-loads/ { ll=$1 } /LLC-load-misses/ { lm=$1 } /L2_RQSTS.ALL/ { l2=$1 } /L2_RQSTS.MISS/" + 
	"{ lm2=$1 } /L3_HITS/ { l3=$1 } END { printf '%f%% ', (l2-lm2)*100/l2; printf '%f%% '" + 
	", lm2*100/l2; printf '%f%% ', (ll-lm)*100/ll; printf '%f%% ', lm*100/ll; printf '%f%%', l3*100/ll }'";
	output = exec(command);
	std::stringstream streamTwo(output);

	streamTwo >> temp;
	processorMetrics.cacheL2HitRate = std::stof(temp);
	streamTwo >> temp;
	processorMetrics.cacheL2MissRate = std::stof(temp);
	streamTwo >> temp;
	processorMetrics.cacheL3HitRate = std::stof(temp);
	streamTwo >> temp;
	processorMetrics.cacheL3MissRate = std::stof(temp);
	streamTwo >> temp;
	processorMetrics.cacheL3HitSnoopRate = std::stof(temp);

	// sudo perf stat -e cpu-cycles,instructions,ref-cycles,cpu-cycles:u,cpu-cycles:u:r0100,cpu-cycles:u:r0200,cpu-cycles:u:r0400,cpu-cycles:u:r0800,cpu-cycles:u:r1000,cpu-cycles:u:r2000,cpu-cycles:u:r4000,cpu-cycles:u:w,cpu-cycles:u:w:r0100,cpu-cycles:u:w:r0200,cpu-cycles:u:w:r0400,cpu-cycles:u:w:r0800,cpu-cycles:u:w:r1000,cpu-cycles:u:w:r2000,cpu-cycles:u:w:r4000 sleep 1 2>&1 | awk '/^cpu-cycles/ { cpu_cycles=$1 } /^instructions/ { instr=$1 } /^ref-cycles/ { ref_cycles=$1 } /^cpu-cycles:u/ { sub(/:/,"_",$1); sub(/u./,"",$1); a[$1]=$1 } /^cpu-cycles:u:/ { sub(/:/,"_",$1); sub(/u./,"",$1); a[$1]=$1 } END { printf "Instructions Retired Rate: %f instructions/cycle\n", instr/cpu_cycles; printf "Processor Cycle Metrics: %f cycles/instruction\n", cpu_cycles/instr; printf "Processor Cycles Reference Rate: %f cycles/second\n", cpu_cycles/ref_cycles; printf "Relative Frequency: %f GHz\n", a["cpu_cycles_u"]/a["cpu_cycles_u_r0100"]/1e9; printf "Active Relative Frequency: %f GHz\n", a["cpu_cycles_u"]/cpu_cycles/1e9 }'
	command = "sudo perf stat -e cpu-cycles,instructions,ref-cycles,cpu-cycles:u,cpu-cycles:u:r0100,cpu-cycles:u:r0200," + 
	"cpu-cycles:u:r0400,cpu-cycles:u:r0800,cpu-cycles:u:r1000,cpu-cycles:u:r2000,cpu-cycles:u:r4000,cpu-cycles:u:w," + 
	"cpu-cycles:u:w:r0100,cpu-cycles:u:w:r0200,cpu-cycles:u:w:r0400,cpu-cycles:u:w:r0800,cpu-cycles:u:w:r1000," + 
	"cpu-cycles:u:w:r2000,cpu-cycles:u:w:r4000 sleep 1 2>&1 | awk '/^cpu-cycles/ { cpu_cycles=$1 } /^instructions/ { instr=$1 }" + 
	" /^ref-cycles/ { ref_cycles=$1 } /^cpu-cycles:u/ { sub(/:/,'_',$1); sub(/u./,'',$1); a[$1]=$1 } /^cpu-cycles:u:/ " + 
	" { sub(/:/,'_',$1); sub(/u./,'',$1); a[$1]=$1 } END { printf '%f ', instr/cpu_cycles; printf '%f ', cpu_cycles/instr;" + 
	" printf '%f ', cpu_cycles/ref_cycles; printf '%f ', a['cpu_cycles_u']/a['cpu_cycles_u_r0100']/1e9; printf '%f', a['cpu_cycles_u']/cpu_cycles/1e9 }'";
	output = exec(command);
	std::stringstream streamThree(output);

	streamThree >> temp;
	processorMetrics.instructionsRetiredRate = std::stof(temp); // instructions/cycle
	streamThree >> temp;
	processorMetrics.cyclesRate = std::stof(temp); 				// cycles/instruction
	streamThree >> temp;
	processorMetrics.cyclesReferenceRate = std::stof(temp); 	// cycles/second
	streamThree >> temp;
	processorMetrics.frequencyRelative = std::stof(temp);		// GHz
	streamThree >> temp;
	processorMetrics.frequencyActiveRelative = std::stof(temp); // GHz
	*/

	processorMetrics.printProcessorMetrics();
};

void getInputOutputMetrics(InputOutputMetrics &inputOutputMetrics){

	const char* command = "sudo awk '{ print $2 }' /proc/1/io";
	std::string output = exec(command), temp;
	std::stringstream stream(output);
	
	stream >> temp;
	inputOutputMetrics.dataRead = std::stoi(temp) / 1024;		// MB
	stream >> temp;
	inputOutputMetrics.dataWritten = std::stoi(temp) / 1024;	// MB
	stream >> temp;
	inputOutputMetrics.readOperationsRate = std::stoi(temp);	// Operations
	stream >> temp;		
	inputOutputMetrics.writeOperationsRate = std::stoi(temp);	// Operations
	stream >> temp;

	command = "iostat -d -k | awk '/^[^ ]/ {device=$1} $1 ~ /sda/ {print $10/$4, $11/$4, $6/$4, $7/$6}'";
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

	inputOutputMetrics.printInputOuputMetrics();
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

	command = "sar -r -B 1 1 | awk 'NR==4{print $3 $4 $5 $7}'";
	output = exec(command);
	std::stringstream streamTwo(output);

	streamTwo >> temp;
	memoryMetrics.pageInRate = std::stoi(temp);			// pages/sec
	streamTwo >> temp;
	memoryMetrics.pageOutRate = std::stoi(temp);		// pages/sec
	streamTwo >> temp;
	memoryMetrics.pageFaultRate = std::stoi(temp);		// pages/sec
	streamTwo >> temp;
	memoryMetrics.pageFreeRate = std::stoi(temp);		// pages/sec

	/*
	// Not supported metrics:
	command = "perf stat -e 'kmem:pgactivate,kmem:pgdeactivate' sleep 1"
	memoryMetrics.pageActivateRate = NOTSUPPORTED;
	memoryMetrics.pageDeactivateRate = NOTSUPPORTED;
	*/

	command = "sar -b 1 1 | awk 'NR==4{print $6/1024 $7/1024 ($6+$7)/1024}'";
	output = exec(command);
	std::stringstream streamThree(output);

	streamThree >> temp;
	memoryMetrics.memoryReadRate = std::stof(temp);		// MB/s
	streamThree >> temp;
	memoryMetrics.memoryWriteRate = std::stof(temp);	// MB/s
	streamThree >> temp;
	memoryMetrics.memoryIoRate = std::stof(temp);		// MB/s
	
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

	command = "cat /proc/net/dev | awk '/^ *eth0:/ {rx=$3; tx=$11; print rx,tx; exit}'";
	output = exec(command);
	std::stringstream streamTwo(output);

	streamTwo >> temp;
	networkMetrics.receivedData = std::stoi(temp);			// number of packets
	streamTwo >> temp;
	networkMetrics.sentData = std::stoi(temp);				// number of packets

	networkMetrics.printNetworkMetrics();
};

void getPowerMetrics(PowerMetrics &powerMetrics){

	/*
	// Not supported metrics:
	powerMetrics.memoryPower = NOTSUPPORTED;
	powerMetrics.processorPower = NOTSUPPORTED;
	powerMetrics.gpuPower = NOTSUPPORTED;
	powerMetrics.gpuPowerHours = NOTSUPPORTED;

	// sudo powerstat -d 1 | awk '/Memory Power/ {printf("Memory Power: %.2f W\n", $4)}'
	const char* command = "sudo powerstat -d 1 | awk '/Memory Power/ { print $4 }'";
	std::string output = exec(command), temp;
	std::stringstream streamOne(output);
	streamOne >> temp;
	powerMetrics.memoryPower = std::stof(temp);		// W

	// [TODO] Test RAPL Library
	rapl_handle_t handle;
	if (rapl_open(&handle) != 0)
		std::cout << "\n\n\t [ERROR] Opening RAPL interface\n";
	else {
		double energy;
		if (rapl_energy_total(handle, &energy) != 0)
			std::cout << "\n\n\t [ERROR] Downloading total energy consumption\n";
		else
			powerMetrics.processorPower = energy;
	}
  	rapl_close(handle);

	// [TODO] Test NVIDIA Library
	nvmlReturn_t result = nvmlInit();
	if (NVML_SUCCESS != result)
		std::cout << "\n\n\t [ERROR] Failed to initialize NVML: " << nvmlErrorString(result) << "\n";
	else {
		// Get the device handle for the first GPU on the system
		nvmlDevice_t device;
		result = nvmlDeviceGetHandleByIndex(0, &device);
		if (NVML_SUCCESS != result) {
			std::cout << "\n\n\t [ERROR] Failed to get handle for GPU 0: " <<  nvmlErrorString(result) << "\n";
			nvmlShutdown();
		}
		else {
			// Get the total energy consumption of the GPU in millijoules
			unsigned long long energyConsumed;
			result = nvmlDeviceGetTotalEnergyConsumption(device, &energyConsumed);
			if (NVML_SUCCESS != result) {
				std::cout << "\n\n\t [ERROR] Failed to get total energy consumption of GPU 0: " << nvmlErrorString(result) << "\n";
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