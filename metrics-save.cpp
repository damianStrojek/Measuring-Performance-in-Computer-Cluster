//
//		metrics-save.cpp - file with definitions of functions related to saving the metrics to a file
//
//		2022-2023	Damian Strojek @damianStrojek
// 					Piotr Garbowski @dideek
// 					Jakub Wasniewski @wisnia01
//

// External libraries
#include <fstream>
// Internal headers
#include "metrics.h"
#include "metrics-save.h"

#define NOTSUPPORTED 666

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