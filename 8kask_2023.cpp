// Gdansk University of Technology 2023
// Group project nr 8 realised at WETI KASK
// Title: An application for monitoring performance and energy consumption in a computing cluster
// Authors: Damian Strojek, Piotr Garbowski, Jakub Wasniewski

#include <iostream>
#include <string>
#include <fstream>

// Constant paths to files in Linux that contain information about different metrics
#define LOADAVG "/prov/loadavg"			// Number of tasks in the queue, process units that are being executed
#define CPUINFO "/proc/cpuinfo"			// Detailed information about each CPU in the system (numbered from 0)
#define PROCSTAT "/proc/stat"			// The amount of time CPU spent performing various types of work
#define INTERRUPTS "/proc/interrupts"	// Counting each system interrupt
#define PROCIO "/proc/$$/io"			// Information about the given task (in this case, task $$).
										// For example: the number of bytes written to and read from memory
#define MEMINFO "/proc/meminfo"			// Operating memory information
#define SWAPINFO "/proc/swap"			// List of the available swap areas

// ---------------------------------------------------------
// Functions that return specific information from each file

// TO-DO TEST THIS SHIT ON LINUX
// Returns all information about load average values in the form of string
std::string loadAverage() {
	std::ifstream procLoadavg;
	// The first three values are the load average values (the number of tasks in the 
	// execution queue or waiting for disk I/O operations) averaged over 1, 5 and 15 minutes.
	// Then there is the number of kernel scheduling units currently executing 
	// and the PID of the process that was last created on the system.
	procLoadavg.open(LOADAVG, std::ios::in);
	if (procLoadavg.is_open()) {
		std::string loadAvg((std::istreambuf_iterator<char>(procLoadavg)),
			std::istreambuf_iterator<char>());
		return loadAvg;
	}
	return "failure";
}

int main() {

	std::cout << "lets start something lol";
	return 0;
}