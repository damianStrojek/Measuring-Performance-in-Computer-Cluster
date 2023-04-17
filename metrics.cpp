//
//		metrics.cpp - file with definitions of functions related to defining and gathering the metrics
//
//		2022-2023	Damian Strojek @damianStrojek
// 					Piotr Garbowski @dideek
// 					Jakub Wasniewski @wisnia01
//

// External libraries
#include <iostream>
#include <string>
// Internal headers
#include <metrics.h>

SystemMetrics::SystemMetrics(){
    this->processesRunning = -1;
    this->processesAll = -1;
    this->processesBlocked = -1;
    this->contextSwitchRate = -1;
    this->interruptRate = -1;
};

void SystemMetrics::printSystemMetrics(){
    std::cout << "\n\t[SYSTEM METRICS]\n\n"
			<< "Interrupt Rate = " << this->interruptRate << " interrupts/sec\n"
			<< "Context Switch Rate = " << this->contextSwitchRate << " switches/sec\n"
			<< "All Processes = " << this->processesAll << "\n"
			<< "Running Processes = " << this->processesRunning << "\n"
			<< "Blocked Processes = " << this->processesBlocked << "\n";
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

ProcessorMetrics::ProcessorMetrics(){
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

void ProcessorMetrics::printProcessorMetrics(){
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

InputOutputMetrics::InputOutputMetrics(){
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

void InputOutputMetrics::printInputOutputMetrics(){
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

MemoryMetrics::MemoryMetrics(){
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

void MemoryMetrics::printMemoryMetrics(){
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

NetworkMetrics::NetworkMetrics(){
    this->receivedData = -1;
    this->receivePacketRate = -1;
    this->sentData = -1;
    this->sendPacketsRate = -1;
};

void NetworkMetrics::printNetworkMetrics(){
    std::cout << "\n\t[NETWORK METRICS]\n\n"
        << "Receive Packet Rate = " << this->receivePacketRate << " KB/s\n"
        << "Send Packet Rate = " << this->sendPacketsRate << " KB/s\n"
        << "Packets Received = " << this->receivedData << "\n"
        << "Packets Sent = " << this->sentData << "\n";
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

PowerMetrics::PowerMetrics(){
    this->coresPower = -1;
    this->processorPower = -1;
    this->memoryPower = -1;
    this->systemPower = -1;
    this->gpuPower = 1;
    this->gpuPowerHours = 1;
};

void PowerMetrics::printPowerMetrics(){
    std::cout << "\n\t[POWER METRICS]\n"
        << "\nCores Power = " << this->coresPower << "W"
        << "\nProcessor = " << this->processorPower << "W"
        << "\nMemory = " << this->memoryPower << "W"
        << "\nSystem = " << this->systemPower << "W"
        << "\nGPU = " << this->gpuPower << "W"
        << "\nGPU = " << this->gpuPowerHours << "Wh\n";
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
	
	powerMetrics.printPowerMetrics();
};

AllMetrics::AllMetrics(){
    this->systemMetrics = SystemMetrics();
    this->processorMetrics = ProcessorMetrics();
    this->inputOutputMetrics = InputOutputMetrics();
    this->memoryMetrics = MemoryMetrics();
    this->networkMetrics = NetworkMetrics();
    this->powerMetrics = PowerMetrics();
};