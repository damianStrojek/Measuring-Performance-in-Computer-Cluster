# Application for Monitoring Performance and Energy Consumption in a Computing Cluster

## Aim

The aim of the project is to create an application in C++ that will allow you to run a given parallel application on a computing cluster and save the values of selected system metrics over time.

## Design Assumptions

The metrics and the places from which they are taken are to be directly specified in the technical documentation of the project. Power measurement is to take place using Intel RAPL, NVIDIA NVML technology and an interface to a professional Yokogawa meter - with a specific frequency. The results should be written to the output of any command line. The results should be saved to text files (.txt, .csv, etc.), and the application should run on a specific distribution of the GNU Linux operating system.

## Requirements

This application requires you to have following tools installed on each of the nodes:

- ifstat
- iostat
- sar
- powerstat
- perf
- vmstat
- Intel RAPL
- NVIDIA Management Library
- mpirun

## Compile

Compile using **C++20** and run using **sudo** permissions:

```bash
# alternatively you can use g++ -std=c++20
mpicxx -std=c++2a measure-performance.cpp metrics.cpp metrics-display.cpp metrics-save.cpp node-synchronization.cpp -o measure-performance
```

Then start it with:

```bash
mpirun -np 2 -mca orte_keep_fqdn_hostnames t -mca btl_tcp_if_exclude docker0,docker_gwbridge,lo -hostfile hostfile.des measure-performance
```

**Make sure that the same version of measure-performance is available on every node that is listed in hostfile.des file.**

If Intel <a href="https://github.com/LLNL/msr-safe" target="_blank">RAPL</a> and <a href="https://developer.nvidia.com/nvidia-management-library-nvml" target="_blank">NVIDIA Management Library</a> is supported:

```bash
mpicxx -std=c++2a measure-performance.cpp metrics.cpp metrics-display.cpp metrics-save.cpp node-synchronization.cpp -o measure-performance -lsmr_safe -lnvidia-ml
```
## MPI

Compile using this instead:

```bash
mpicxx measure-performance.cpp -o measure-performance
```

Then start it with:
```bash
mpirun -np 2 -mca orte_keep_fqdn_hostnames t -mca btl_tcp_if_exclude docker0,docker_gwbridge,lo -hostfile hostfile.des measure-performance
```
Make sure measure-performance is available on every node that is listed in hostfile.des file.
## Docker

How to run docker environment:

```bash
docker run -it --mount type=bind,source="$(pwd)",target=/app kask-env
```

## Contributors

- [dStrojek [Project Manager, Developer]](https://github.com/damianStrojek) - Working on team workflow, fetching metrics, overall style of the code and repositorium. Creator of documentation.

- [wisnia [Developer]](https://github.com/wisnia01) - Working on print and save to file functions.

- [dideek [Developer]](https://github.com/dideek) - Working on synchronization between nodes in computer claster.
