//
//      metrics-display.cpp - file with definitions of functions related to displaying the metrics
//
//      2022-2023	Damian Strojek @damianStrojek
// 		    		Piotr Garbowski @dideek
// 		    		Jakub Wasniewski @wisnia01
//

// External libraries
#include <iostream>
#include <string>
// Internal headers
#include <metrics.h>
#include <metrics-display.h>

void printMetric(std::string metricName, int metricValue, std::string metricUnit){

	std::string value = std::to_string(metricValue);
	if(metricValue == NOTSUPPORTED) {
		value = "notSupported";
		metricUnit = "";
	}
	std::cout << metricName << std::right << std::setfill('.') << std::setw(30-metricName.length()) << value << metricUnit << "\n";

    return;
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

    return;
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

    return;
};