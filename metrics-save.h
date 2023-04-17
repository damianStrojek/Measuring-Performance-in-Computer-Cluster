//
//		metrics-save.h - header file for functions related to saving the metrics to a file
//
//		2022-2023	Damian Strojek @damianStrojek
// 					Piotr Garbowski @dideek
// 					Jakub Wasniewski @wisnia01
//

#ifndef METRICS_SAVE_H
#define METRICS_SAVE_H

// External libraries
#include <fstream>
// Internal headers
#include <metrics.h>

// Write to file functions
void writeToFileSystemMetrics(std::ofstream&, SystemMetrics);
void writeToFileProcessorMetrics(std::ofstream&, ProcessorMetrics);
void writeToFileInputOutputMetrics(std::ofstream&, InputOutputMetrics);
void writeToFileMemoryMetrics(std::ofstream&, MemoryMetrics);
void writeToFileNetworkMetrics(std::ofstream&, NetworkMetrics);
void writeToCSV(std::ofstream&, std::string, SystemMetrics, ProcessorMetrics, 
					InputOutputMetrics, MemoryMetrics, NetworkMetrics);

#endif