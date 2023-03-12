# Application for Monitoring Performance and Energy Consumption in a Computing Cluster

## Aim

The aim of the project is to create an application in C++ that will allow you to run a given application (parallel) on a computing cluster and save the values of selected metrics over time.

## Design Assumptions

The metrics and the places from which they are taken are to be directly specified in the technical documentation of the project. Power measurement is to take place using Intel RAPL, NVIDIA NVML technology and an interface to a professional Yokogawa meter - with a specific frequency. The results should be written to the output of any command line. The results should be saved to text files (.txt, .csv, etc.), and the application should run on a specific distribution of the GNU Linux operating system.

## Requirements and Compile

This application requires you to have following tools installed on each of the nodes:

- ifstat
- iostat
- sar
- powerstat
- perf
- vmstat
- Intel RAPL
- NVIDIA Management Library

Compile using **C++20** and run using **sudo** permissions:

```bash
g++ -std=c++20 -o measure-performance measure-performance.cpp
```

## Docker

How to run docker environment:

```bash
docker run -it --mount type=bind,source="$(pwd)",target=/app kask-env
```

## Contributors

- [Me - project manager](https://github.com/damianStrojek)

- [Wisnia - developer](https://github.com/wisnia01)

- [Pitya - developer](https://github.com/dideek)
