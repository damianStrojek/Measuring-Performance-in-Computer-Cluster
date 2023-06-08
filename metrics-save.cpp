//
//	metrics-save.cpp - file with definitions of functions related to saving the metrics to a file
//
//	2022-2023	Damian Strojek @damianStrojek
// 			Piotr Garbowski @dideek
// 			Jakub Wasniewski @wisnia01
//

// External libraries
#include <fstream>
#include "json.hpp"
#include <chrono>
#include <sstream>
// Internal headers
#include "metrics.h"
#include "metrics-save.h"

using json = nlohmann::json;

json metricsToJson(AllMetrics* allMetricsArray,int cluster_size){
	
	json jsonSingleIteration;
	auto now = std::chrono::system_clock::now();
  	std::time_t now_c = std::chrono::system_clock::to_time_t(now);
	std::stringstream ss;
	ss << std::put_time(std::localtime(&now_c), "%Y-%m-%d %X");
	jsonSingleIteration["_timestamp"] = ss.str();
	json nodes = json::array();
	for(int i = 0; i < cluster_size ; i++){
	AllMetrics allMetrics = allMetricsArray[i];
	// create JSON objects for the sub-structures
	json systemMetricsJSON = {
		{"processesRunning", allMetrics.systemMetrics.processesRunning},
		{"processesAll", allMetrics.systemMetrics.processesAll},
		{"processesBlocked", allMetrics.systemMetrics.processesBlocked},
		{"contextSwitchRate", allMetrics.systemMetrics.contextSwitchRate},
		{"interruptRate", allMetrics.systemMetrics.interruptRate}
	};

	json processorMetricsJSON = {
		{"timeUser", allMetrics.processorMetrics.timeUser},
		{"timeNice", allMetrics.processorMetrics.timeNice},
		{"timeSystem", allMetrics.processorMetrics.timeSystem},
		{"timeIdle", allMetrics.processorMetrics.timeIdle},
		{"timeIoWait", allMetrics.processorMetrics.timeIoWait},
		{"timeIRQ", allMetrics.processorMetrics.timeIRQ},
		{"timeSoftIRQ", allMetrics.processorMetrics.timeSoftIRQ},
		{"timeSteal", allMetrics.processorMetrics.timeSteal},
		{"timeGuest", allMetrics.processorMetrics.timeGuest},
		{"instructionsRetired", allMetrics.processorMetrics.instructionsRetired},
		{"cycles", allMetrics.processorMetrics.cycles},
		{"frequencyRelative", allMetrics.processorMetrics.frequencyRelative},
		{"unhaltedFrequency", allMetrics.processorMetrics.unhaltedFrequency},
		{"cacheL2Misses", allMetrics.processorMetrics.cacheL2Misses},
		{"cacheL2Requests", allMetrics.processorMetrics.cacheL2Requests},
		{"cacheLLCLoadMisses", allMetrics.processorMetrics.cacheLLCLoadMisses},
		{"cacheLLCLoadMissRate", allMetrics.processorMetrics.cacheLLCLoadMissRate},
		{"cacheLLCLoads", allMetrics.processorMetrics.cacheLLCLoads},
		{"cacheLLCStoreMisses", allMetrics.processorMetrics.cacheLLCStoreMisses},
		{"cacheLLCStoreMissRate", allMetrics.processorMetrics.cacheLLCStoreMissRate},
		{"cacheLLCStores", allMetrics.processorMetrics.cacheLLCStores}
	};

	json inputOutputMetricsJSON = {
		{"processID", allMetrics.inputOutputMetrics.processID},
		{"dataRead", allMetrics.inputOutputMetrics.dataRead},
		{"readTime", allMetrics.inputOutputMetrics.readTime},
		{"readOperationsRate", allMetrics.inputOutputMetrics.readOperationsRate},
		{"dataWritten", allMetrics.inputOutputMetrics.dataWritten},
		{"writeTime", allMetrics.inputOutputMetrics.writeTime},
		{"writeOperationsRate", allMetrics.inputOutputMetrics.writeOperationsRate},
		{"flushTime", allMetrics.inputOutputMetrics.flushTime},
		{"flushOperationsRate", allMetrics.inputOutputMetrics.flushOperationsRate}
	};

	json memoryMetricsJSON = {
		{"memoryUsed", allMetrics.memoryMetrics.memoryUsed},
		{"memoryCached", allMetrics.memoryMetrics.memoryCached},
		{"swapUsed", allMetrics.memoryMetrics.swapUsed},
		{"swapCached", allMetrics.memoryMetrics.swapCached},
		{"memoryActive", allMetrics.memoryMetrics.memoryActive},
		{"memoryInactive", allMetrics.memoryMetrics.memoryInactive},
		{"pageInRate", allMetrics.memoryMetrics.pageInRate},
		{"pageOutRate", allMetrics.memoryMetrics.pageOutRate},
		{"pageFaultRate", allMetrics.memoryMetrics.pageFaultRate},
		{"pageFaultsMajorRate", allMetrics.memoryMetrics.pageFaultsMajorRate},
		{"pageFreeRate", allMetrics.memoryMetrics.pageFreeRate},
		{"pageActivateRate", allMetrics.memoryMetrics.pageActivateRate},
		{"pageDeactivateRate", allMetrics.memoryMetrics.pageDeactivateRate},
		{"memoryReadRate", allMetrics.memoryMetrics.memoryReadRate},
		{"memoryWriteRate", allMetrics.memoryMetrics.memoryWriteRate},
		{"memoryIoRate", allMetrics.memoryMetrics.memoryIoRate}
	};

	json networkMetricsJSON = {
		{"receivedData", allMetrics.networkMetrics.receivedData},
		{"receivePacketRate", allMetrics.networkMetrics.receivePacketRate},
		{"sentData", allMetrics.networkMetrics.sentData},
		{"sendPacketsRate", allMetrics.networkMetrics.sendPacketsRate}
	};

	json powerMetricsJSON = {
		{"processorPower", allMetrics.powerMetrics.processorPower},
		{"memoryPower", allMetrics.powerMetrics.memoryPower},
		{"systemPower", allMetrics.powerMetrics.systemPower},
		{"gpuPower", allMetrics.powerMetrics.gpuPower},
		{"gpuPowerHours", allMetrics.powerMetrics.gpuPowerHours}
	};

	json allMetricsJSON = {
		//{"nodeTimestamp", allMetrics.nodeTimestamp},
		{"systemMetrics", {systemMetricsJSON}},
		{"processorMetrics", {processorMetricsJSON}},
		{"inputOutputMetrics", {inputOutputMetricsJSON}},
		{"memoryMetrics", {memoryMetricsJSON}},
		{"networkMetrics", {networkMetricsJSON}},
		{"powerMetrics", {powerMetricsJSON}}
	};
	json singleNode;
	singleNode["_Node"] = i;
	singleNode["Metrics"] = allMetricsJSON;
	nodes.push_back(singleNode);
	}
	jsonSingleIteration["Nodes"] = nodes;
	// Save the JSON line into a file
	return jsonSingleIteration;
};