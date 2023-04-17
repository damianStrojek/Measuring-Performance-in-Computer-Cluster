//
//	metrics-display.h - header file for functions related to displaying the metrics
//
//	2022-2023	Damian Strojek @damianStrojek
// 			Piotr Garbowski @dideek
// 			Jakub Wasniewski @wisnia01
//

#ifndef METRICS_DISPLAY_H
#define METRICS_DISPLAY_H

// Internal headers
#include "metrics.h"

// Printing for the user
void printMetric(std::string, int, std::string);
void printMetricPair(std::string, int, std::string, std::string, int, std::string);
void printMetrics(SystemMetrics*, ProcessorMetrics*, InputOutputMetrics*, 
			MemoryMetrics*, NetworkMetrics*);

#endif