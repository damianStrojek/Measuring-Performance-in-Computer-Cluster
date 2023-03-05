# Application for monitoring performance and energy consumption in a computing cluster

## Aim

The aim of the project is to create an application in C++ that will allow you to run a given application (parallel) on a computing cluster and save the values of selected metrics over time.

## Design assumptions

The metrics and the places from which they are taken are to be directly specified in the technical documentation of the project. Power is to be measured using Intel RAPL, NVIDIA NVML technology and an interface to the professional Yokogawa meter - with a predetermined frequency. Results are written to the output of any command line. The results are to be saved in text files, and the operation of the application is to be carried out for a specific distribution of the Linux operating system.

## Requirements and compile

This application requires you to have:

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
g++ -std=c++20 -o metrics 8kask_2023.cpp
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
