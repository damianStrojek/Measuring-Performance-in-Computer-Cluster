//
// 	measure-performance.cpp - main file of a project
//
// 	2022-2023	Damian Strojek @damianStrojek
// 			Piotr Garbowski @dideek
// 			Jakub Wasniewski @wisnia01
//
// An application for monitoring performance and energy consumption in a computing cluster
//
// mpicxx -std=c++2a measure-performance.cpp metrics.cpp metrics-display.cpp metrics-save.cpp node-synchronization.cpp -o measure-performance
//
// Project realised in years 2022-2023 on
// Gdansk University of Technology, Department of Computer Systems Architecture
// 

// External libraries
#include <iostream>
#include <string>
#include <cstring>
#include <stdexcept>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <mpi.h>
// Internal headers
#include "metrics.h"
#include "metrics-save.h"
#include "metrics-display.h"
#include "node-synchronization.h"

#define GPROCESSID 1				// PID of process that we are focused on (G stands for global)

int keyboardHit(void);

int main(int argc, char **argv){

	SystemMetrics systemMetrics;
	ProcessorMetrics processorMetrics;
	InputOutputMetrics inputOutputMetrics;
	MemoryMetrics memoryMetrics;
	NetworkMetrics networkMetrics;
	PowerMetrics powerMetrics;

	bool raplError = 0, nvmlError = 0;
	const char* dateCommand = "date +'%d%m%y-%H%M%S'",
		*processCommand = "ps -p 1 > /dev/null && echo '1' || echo '0'";	// [TODO] Add GPROCESSID
	std::string timestamp = exec(dateCommand);
	timestamp.pop_back();

	/*std::string fileName = timestamp += "_metrics.csv";
	std::ofstream file(fileName, std::ios::out);
	if(!file.is_open()) std::cerr << "\n\n\t [ERROR] Unable to open file " << fileName << " for writing.\n";*/

	int rank, size;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	AllMetrics allMetrics;
	MPI_Datatype systemMetricsType = createMpiSystemMetricsType();
	MPI_Datatype processorMetricsType = createMpiProcessorMetricsType();
	MPI_Datatype inputOutputMetricsType = createMpiInputOutputMetricsType();
	MPI_Datatype memoryMetricsType = createMpiMemoryMetricsType();
	MPI_Datatype networkMetricsType = createMpiNetworkMetricsType();
	MPI_Datatype powerMetricsType = createMpiPowerMetricsType();
	MPI_Datatype allMetricsType;

	int blocklengths[] = {1, 1, 1, 1, 1, 1};
    	MPI_Datatype types[] = {
		systemMetricsType, processorMetricsType, inputOutputMetricsType,
		memoryMetricsType, networkMetricsType, powerMetricsType};
    	MPI_Aint offsets[] = {
		offsetof(struct AllMetrics, systemMetrics),
		offsetof(struct AllMetrics, processorMetrics),
		offsetof(struct AllMetrics, inputOutputMetrics),
		offsetof(struct AllMetrics, memoryMetrics),
		offsetof(struct AllMetrics, networkMetrics),
		offsetof(struct AllMetrics, powerMetrics)};

	MPI_Type_create_struct(6, blocklengths, offsets, types, &allMetricsType);
	MPI_Type_commit(&allMetricsType);
	AllMetrics* allMetricsArray = new AllMetrics[size];

	// Checking if process with GPROCESSID is still running
	while(std::stoi(exec(processCommand))){

		if(keyboardHit()){
				std::cout << "\n\n\t[STOP] Key pressed.\n\n";
				break;
			}	
			
			timestamp = exec(dateCommand);
			timestamp.pop_back();
			std::cout << "\n\n   [TIMESTAMP] " << timestamp << "\n";

			//auto start = std::chrono::high_resolution_clock::now();

			getSystemMetrics(allMetrics.systemMetrics);
			getProcessorMetrics(allMetrics.processorMetrics);
			getInputOutputMetrics(allMetrics.inputOutputMetrics);
			getMemoryMetrics(allMetrics.memoryMetrics);
			getNetworkMetrics(allMetrics.networkMetrics);
			getPowerMetrics(allMetrics.powerMetrics, raplError, nvmlError);
		
			if(rank)
				MPI_Send(&allMetrics, 1, allMetricsType, 0, 0, MPI_COMM_WORLD);
			else {
				allMetricsArray[0] = allMetrics;
				for(int i = 1; i < size; i++)
					MPI_Recv(&allMetricsArray[i], 1, allMetricsType, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				for(int i = 0;i < size; i++){
					std::cout << "\n\t[PROCESS " << i << " METRICS]\n";
					printMetrics(&allMetricsArray[i].systemMetrics, &allMetricsArray[i].processorMetrics, \
							&allMetricsArray[i].inputOutputMetrics, &allMetricsArray[i].memoryMetrics, \
							&allMetricsArray[i].networkMetrics);
				}
			}

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
	MPI_Type_free(&systemMetricsType);
	MPI_Type_free(&processorMetricsType);
	MPI_Type_free(&inputOutputMetricsType);
	MPI_Type_free(&memoryMetricsType);
	MPI_Type_free(&networkMetricsType);
	MPI_Type_free(&powerMetricsType);
	MPI_Type_free(&allMetricsType);
	delete[] allMetricsArray;
   	MPI_Finalize();
	return 0;
};

// Actively checking for the user input to break from the main while loop
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
