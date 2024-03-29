//
//      node-synchronization.cpp - file with definitions of functions related to synchronizing metrics gathered from different nodes
//
//      2022-2023	Damian Strojek @damianStrojek
// 				    Piotr Garbowski @dideek
// 				    Jakub Wasniewski @wisnia01
//

// Internal headers
#include "node-synchronization.h"
#include "metrics.h"

// Create MPI data type for SystemMetricsType
MPI_Datatype createMpiSystemMetricsType(){

    int blockLengths[] = {1, 1, 1, 1, 1};
    MPI_Datatype metricTypes[] = {
        MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT};
    MPI_Aint metricOffsets[] = {
        offsetof(struct SystemMetrics, processesRunning),
        offsetof(struct SystemMetrics, processesAll),
        offsetof(struct SystemMetrics, processesBlocked),
        offsetof(struct SystemMetrics, contextSwitchRate),
        offsetof(struct SystemMetrics, interruptRate)};

    MPI_Datatype systemMetricsType;
    MPI_Type_create_struct(5, blockLengths, metricOffsets, metricTypes, &systemMetricsType);
    MPI_Type_commit(&systemMetricsType);

    return systemMetricsType;
};

// Create MPI data type for ProcessorMetricsType
MPI_Datatype createMpiProcessorMetricsType(){

    int blockLengths[] = {
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1};
    MPI_Datatype metricTypes[] = {
        MPI_INT, MPI_INT, MPI_INT, MPI_INT, 
        MPI_INT, MPI_INT, MPI_INT, MPI_INT, 
        MPI_INT, MPI_INT, MPI_INT, MPI_FLOAT, 
        MPI_FLOAT, MPI_INT, MPI_INT, MPI_INT, 
        MPI_INT, MPI_INT, MPI_FLOAT, MPI_INT,
        MPI_FLOAT};
    MPI_Aint metricOffsets[] = {
        offsetof(struct ProcessorMetrics, timeUser),
        offsetof(struct ProcessorMetrics, timeNice),
        offsetof(struct ProcessorMetrics, timeSystem),
        offsetof(struct ProcessorMetrics, timeIdle),
        offsetof(struct ProcessorMetrics, timeIoWait),
        offsetof(struct ProcessorMetrics, timeIRQ),
        offsetof(struct ProcessorMetrics, timeSoftIRQ),
        offsetof(struct ProcessorMetrics, timeSteal),
        offsetof(struct ProcessorMetrics, timeGuest),
        offsetof(struct ProcessorMetrics, instructionsRetired),
        offsetof(struct ProcessorMetrics, cycles),
        offsetof(struct ProcessorMetrics, frequencyRelative),
        offsetof(struct ProcessorMetrics, unhaltedFrequency),
        offsetof(struct ProcessorMetrics, cacheL2Requests),
        offsetof(struct ProcessorMetrics, cacheL2Misses),
        offsetof(struct ProcessorMetrics, cacheLLCLoads),
        offsetof(struct ProcessorMetrics, cacheLLCStores),
        offsetof(struct ProcessorMetrics, cacheLLCLoadMisses),
        offsetof(struct ProcessorMetrics, cacheLLCLoadMissRate),
        offsetof(struct ProcessorMetrics, cacheLLCStoreMisses),
        offsetof(struct ProcessorMetrics, cacheLLCStoreMissRate)};

    MPI_Datatype processorMetricsType;
    MPI_Type_create_struct(21, blockLengths, metricOffsets, metricTypes, &processorMetricsType);
    MPI_Type_commit(&processorMetricsType);

    return processorMetricsType;
};

// Create MPI data type for InputOutputMetricsType
MPI_Datatype createMpiInputOutputMetricsType(){
    
    int blockLengths[] = {1, 1, 1, 1, 1, 1, 1, 1, 1};
    MPI_Datatype metricTypes[] = {
        MPI_INT, MPI_FLOAT, MPI_FLOAT, 
        MPI_INT, MPI_FLOAT, MPI_FLOAT, 
        MPI_INT, MPI_FLOAT, MPI_FLOAT};
    MPI_Aint metricOffsets[] = {
        offsetof(struct InputOutputMetrics, processID),
        offsetof(struct InputOutputMetrics, dataRead),
        offsetof(struct InputOutputMetrics, readTime),
        offsetof(struct InputOutputMetrics, readOperationsRate),
        offsetof(struct InputOutputMetrics, dataWritten),
        offsetof(struct InputOutputMetrics, writeTime),
        offsetof(struct InputOutputMetrics, writeOperationsRate),
        offsetof(struct InputOutputMetrics, flushTime),
        offsetof(struct InputOutputMetrics, flushOperationsRate)};

    MPI_Datatype inputOutputMetricsType;
    MPI_Type_create_struct(9, blockLengths, metricOffsets, metricTypes, &inputOutputMetricsType);
    MPI_Type_commit(&inputOutputMetricsType);

	return inputOutputMetricsType;
};


// Create MPI data type for MemoryMetricsType
MPI_Datatype createMpiMemoryMetricsType(){

    int blockLengths[] = {
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    MPI_Datatype metricTypes[] = {
        MPI_FLOAT, MPI_FLOAT, MPI_FLOAT, MPI_FLOAT, 
        MPI_FLOAT, MPI_FLOAT, MPI_FLOAT, MPI_FLOAT, 
        MPI_FLOAT, MPI_FLOAT, MPI_FLOAT, MPI_FLOAT,
        MPI_FLOAT, MPI_FLOAT, MPI_FLOAT, MPI_FLOAT};
    MPI_Aint metricOffsets[] = {
        offsetof(struct MemoryMetrics, memoryUsed),
        offsetof(struct MemoryMetrics, memoryCached),
        offsetof(struct MemoryMetrics, swapUsed),
        offsetof(struct MemoryMetrics, swapCached),
        offsetof(struct MemoryMetrics, memoryActive),
        offsetof(struct MemoryMetrics, memoryInactive),
        offsetof(struct MemoryMetrics, pageInRate),
        offsetof(struct MemoryMetrics, pageOutRate),
        offsetof(struct MemoryMetrics, pageFaultRate),
        offsetof(struct MemoryMetrics, pageFaultsMajorRate),
        offsetof(struct MemoryMetrics, pageFreeRate),
        offsetof(struct MemoryMetrics, pageActivateRate),
        offsetof(struct MemoryMetrics, pageDeactivateRate),
        offsetof(struct MemoryMetrics, memoryReadRate),
        offsetof(struct MemoryMetrics, memoryWriteRate),
        offsetof(struct MemoryMetrics, memoryIoRate)};

    MPI_Datatype memoryMetricsType;
    MPI_Type_create_struct(16, blockLengths, metricOffsets, metricTypes, &memoryMetricsType);
    MPI_Type_commit(&memoryMetricsType);

    return memoryMetricsType;
};

// Create MPI data type for NetworkMetricsType
MPI_Datatype createMpiNetworkMetricsType(){

    int blockLengths[] = {1, 1, 1, 1};
    MPI_Datatype metricTypes[] = {
        MPI_INT, MPI_FLOAT, MPI_INT, MPI_FLOAT};
    MPI_Aint metricOffsets[] = {
        offsetof(NetworkMetrics, receivedData),
        offsetof(NetworkMetrics, receivePacketRate),
        offsetof(NetworkMetrics, sentData),
        offsetof(NetworkMetrics, sendPacketsRate)};

    MPI_Datatype networkMetricsType;
    MPI_Type_create_struct(4, blockLengths, metricOffsets, metricTypes, &networkMetricsType);
    MPI_Type_commit(&networkMetricsType);

    return networkMetricsType;
};

// Create MPI data type for PowerMetrics
MPI_Datatype createMpiPowerMetricsType(){

    int blockLengths[] = {
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    MPI_Datatype metricTypes[] = {
        MPI_FLOAT, MPI_FLOAT, MPI_FLOAT, MPI_FLOAT,
        MPI_FLOAT, MPI_FLOAT, MPI_FLOAT, MPI_FLOAT,
        MPI_FLOAT, MPI_FLOAT, MPI_FLOAT};
    MPI_Aint metricOffsets[] = {
        offsetof(PowerMetrics, processorPower),
        offsetof(PowerMetrics, memoryPower),
        offsetof(PowerMetrics, systemPower),
        offsetof(PowerMetrics, gpuPower),
        offsetof(PowerMetrics, gpuTemperature),
        offsetof(PowerMetrics, gpuFanSpeed),
        offsetof(PowerMetrics, gpuMemoryTotal),
        offsetof(PowerMetrics, gpuMemoryUsed),
        offsetof(PowerMetrics, gpuMemoryFree),
        offsetof(PowerMetrics, gpuClocksCurrentSM),
        offsetof(PowerMetrics, gpuClocksCurrentMemory)};
    
    MPI_Datatype powerMetricsType;
    MPI_Type_create_struct(11, blockLengths, metricOffsets, metricTypes, &powerMetricsType);
    MPI_Type_commit(&powerMetricsType);

    return powerMetricsType;
};