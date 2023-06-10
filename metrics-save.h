//
//	metrics-save.h - header file for functions related to saving the metrics to a file
//
//	2022-2023	Damian Strojek @damianStrojek
// 			Piotr Garbowski @dideek
// 			Jakub Wasniewski @wisnia01
//

#ifndef METRICS_SAVE_H
#define METRICS_SAVE_H

// External libraries
#include <fstream>      // ifstream, ofstream
// Internal headers
#include "metrics.h"
#include "json.hpp"

// Write to file function
nlohmann::json metricsToJson(AllMetrics*, int);

#endif