# Metric Fetching Functions

The purpose of writing this documentation is to show the logic behind used Linux commands. Many of these metrics can be obtained in several ways. The methods used in this application have been selected based on several criteria:

- Availability from the level of a "clean" system based on GNU Linux,
- No need to wait for results to be calculated, e.g., we prefer to read metrics from the files,
- Instead of reading different information from 5 different places, we prefer to find a place from which we will be able to download most of these metrics and then possibly supplement them.

## List of paths and tools

Below is a list of paths from which we fetched data:

- `/proc/loadavg`
- `/proc/cpuinfo`
- `/proc/stat`
- `/proc/<gPROCESSID>/io`
- `/proc/meminfo`
- `/proc/net/dev`

And list of tools/commands:

- date
- vmstat
- ps
- iostat
- sar
- ifstat
- powerstat
- cut, grep, cat, awk, tail

## System Metrics

### Interrupt and Context Switch Rates

```bash
vmstat
```

As far as we know, the output of the `vmstat` command has fixed places in which the metric data is entered (i.e. there is no shift if a given metric at some point takes 1,2,3 places more than before). For this reason, we could specify fixed places from which we read the interrupt rate and context switch rate.

![Output](./images/interrupt-context-rates.png)

### Number of All and Running Processes

```bash
cat /proc/loadavg | cut -d ' ' -f 4
```

From the file `/proc/loadavg` we only get the number of all processes and processes that are currently running. This number is separated by a slash (`/`).

![Output](./images/all-and-running-processes.png)

### Number of Blocked Processes

```bash
ps -eo state | grep -c '^D'
```

Blocked processes are not available using previous methods so we had to use another command to get this number. This is a possibility of improvement in the future.

![Output](./images/blocked-processes.png)

## Processor Metrics

### Number of Processors

```bash
cat /proc/cpuinfo | grep 'processor' -c
```

Getting the number of processors that were reported to the `/proc/cpuinfo`. We can base our algorithm on that and read specific information about each processor.

![Output](./images/number-of-processors.png)

### User, Nice, System, Idle, [...] Times

```bash
cat /proc/stat
```

This file gives us a lot of information about all of the processors. We are only interested in the first line of the output which provides sum of information from all processors reported to the `/proc/cpuinfo`.

All of the times are measured in `USER_HZ` which is typically 1/100 of a second.

![Output](./images/processor-times.png)

### L2 and LLC Cache Hit and Miss Rates

```bash
perf stat -e "l2_rqsts.references,l2_rqsts.miss,LLC-loads,LLC-stores,LLC-load-misses,LLC-store-misses" --all-cpus sleep 1 2>&1 | awk '/^[ ]*[0-9]/{print $1}'
```

`l2_rqsts.references`: This metric represents the number of L2 cache requests issued by the processor. It measures the total number of times the processor accessed the L2 cache.

`l2_rqsts.miss`: This metric represents the number of L2 cache misses. It measures the number of times the processor requested data from the L2 cache but found that the data was not present, resulting in a cache miss.

`LLC-loads`: This metric represents the number of cache loads from the Last Level Cache (LLC). It measures the total number of times the processor requested data from the LLC.

`LLC-stores`: This metric represents the number of cache stores to the LLC. It measures the total number of times the processor wrote data to the LLC.

`LLC-load-misse`: This metric represents the number of LLC load misses. It measures the number of times the processor requested data from the LLC but found that the data was not present, resulting in a cache miss.

`LLC-store-misses` This metric represents the number of LLC store misses. It measures the number of times the processor attempted to write data to the LLC but found that the cache line for the data was not present, resulting in a cache miss.

These metrics provide insights into the cache behavior of the processor during the execution of the workload. Cache hits are desirable as they indicate that the data is readily available in the cache, improving performance. Cache misses, on the other hand, result in additional latency as the processor needs to retrieve the data from a higher-level cache or main memory.

*Note that these formulas assume that the events provided in the perf stat command accurately measure the cache behavior of your system. However, depending on the specific hardware and software configuration, the event names or formulas may need to be adjusted.*

![Output](./images/processor-l2-llc-rates.png)

### Cycles Rates and Relative Frequencies

```bash
perf stat -e instructions,cycles,cpu-clock,cpu-clock:u sleep 1 2>&1 | awk '/^[ ]*[0-9]/{print $1}'
```

- `Number of instructions retired`: This is the total number of instructions executed by the processor during the sampling period.

- `Cycles`: This is the total number of cycles executed by the processor during the sampling period.

- `CPU clock`: This is the CPU clock frequency in MHz during the sampling period.

- `CPU clock:u`: This is the unhalted CPU clock frequency in MHz during the sampling period.

![Output](./images/processor-clocks.png)

## Input / Output Metrics

### Data read and written, Read and Write Operations Rates

```bash
sudo awk '{ print $2 }' /proc/1/io
```

Basically this command outputs second column of the `/proc/1/io file`. The `1` in the filepath is the Process ID (it is defined globally as `GPROCESSID`). 

The first and second rows are respectively all characters read and written by the specified process divided by 1024 to get this number in MB. Right now this command doesn't count write and read "operations rate". It just outputs the number of read and write operations for this specific process ID.

![Output](./images/io-data-read-written.png)

### Read, Write, Flush operations and Flush time

```bash
iostat -d -k | awk '/^[^ ]/ {device=$1} $1 ~ /sda/ {print 1000*$10/($4*$3), 1000*$11/($4*$3), $6/$4, $7/$6}'

/^[^ ]/ {device=$1}              # If the line starts with a non-space character, set the variable "device" to the first field
$1 ~ /sda/ {                     # If the first field contains "sda"
    print 1000*$10/($4*$3),      # Print the read time in milliseconds (Field 10 * 1000 / (Field 4 * Field 3))
          1000*$11/($4*$3),      # Print the write time in milliseconds (Field 11 * 1000 / (Field 4 * Field 3))
          $6/$4,                 # Print the flush operations per second (Field 6 / Field 4)
          $7/$6                  # Print the flush time per flush operation in milliseconds (Field 7 / Field 6)
}
```

This command will first run `iostat -d -k` to get the disk statistics, then use `awk` to parse the output. The awk script looks for lines that start with a non-space character (which indicates the start of a new device's statistics) and saves the device name to the device variable. It then looks for lines that contain the string sda (*which can be replaced with any other device name as needed*) and calculates all of the metrics.

![Output](./images/io-read-write-times.png)

## Memory Metrics

### Active, Inactive, Cached, and Used Memory and Swap

```bash
grep -v -e 'anon' -e 'file' /proc/meminfo | grep -E '^(Cached|SwapCached|SwapTotal|SwapFree|Active|Inactive)' | awk '{print $2}'
```

All of the information we can get from the `/proc/meminfo` file. All of the metrics are saved in kB but, for the sake of the output, we are dividing it by 1024 to have MB.

![Output](./images/memory-swap-active-cached.png)

### Page In, Out, Fault, Free, Activate and Deactivate Rates

```bash
sar -r -B 1 1 | awk 'NR==4{print $2,$3,$4,$5,$6,$7,$8}'
```

This command uses the `-r` and `-B` options to collect memory page and paging statistics, and then selects the relevant fields using `awk`. All of the statistics for pages are measured in pages/second.

The `pgscank/s` column shows the rate of kernel page scans in kilopages per second, and the `pgscand/s` column shows the rate of direct reclaim attempts in kilopages per second.

![Output](./images/page-in-out.png)

### Memory Read, Write, and I/O Rates

```bash
sar -b 1 1 | awk 'NR==4{print $6/1024,$7/1024,($6+$7)/1024}'
```

This command selects the fourth line of the `sar -b` output, which contains the memory read and write rates in kilobytes per second. The awk command then divides these rates by 1024 to convert them to megabytes per second, and prints the results with descriptive labels. Finally, it calculates the total memory I/O rate by summing the memory read and write rates, and also converts the result to MB/s.

Note that the output of this command may vary depending on your Linux distribution and version, so you may need to adjust the awk commands accordingly to extract the desired metrics.

![Output](./images/memory-read-write-rates.png)

## Network Metrics

### Receive and Send Packet Rates

```bash
ifstat 1 1 | tail -1 | awk '{ print $1, $2 }'
```

Unfortunately, I haven't been able to find a place from which I could download information about the current receive and send packet rate that would not require a 1 second delay from me to measure it.

Both metrics are measured in KB/s.

![Output](./images/receive-and-send-packet-rates.png)

### Sent and Received Data

```bash
cat /proc/net/dev | awk '/^ *eth0:/ {rx=$3; tx=$11; print rx,tx; exit}'
```

The file `/proc/net/dev` keeps track of all received and sent packets and bytes. Right now we are outputting number of packets but it's possible to change the command and output number of bytes.

**There might be a need to change `eth0` to appropriate name of the network interface.**

![Output](./images/sent-and-received-data.png)

## Power Metrics

### Power not using RAPL or NVML

```bash
perf stat -e power/energy-cores/,power/energy-ram/,power/energy-pkg/ sleep 1 2>&1 | awk '/Joules/ {print $1}'
```

- `power/energy-cores/`: This measures the energy consumed by the CPU cores during the measurement period. The CPU cores are the primary consumers of power in most computing systems, as they perform the majority of the processing work.

- `power/energy-ram/`: This measures the energy consumed by the RAM (memory) during the measurement period. RAM is a significant consumer of power, as it needs to constantly refresh its memory cells to maintain data integrity.

- `power/energy-pkg/`: This measures the energy consumed by the entire package, which includes the CPU, RAM, and other components such as the chipset and power delivery system. The power/energy-pkg/ metric is generally the most relevant metric for measuring the overall power consumption of a computing system, as it captures the energy consumed by all of the major components.

![Output](./images/power-consumption-perf.png)

Regarding `powerstat` - it needs to be run with root privilege when using `-g`, `-p`, `-r`, `-s` options.`

### Using RAPL

```c++
if(!raplError){
    double energy;
    if (rapl_get_energy(RAPL_PACKAGE, &energy) != 0){
        std::cerr << "\n\n\t[ERROR] Failed to get package energy consumption\n";
        raplError = 1;
        rapl_finish();
    }
    else
        powerMetrics.processorPower = energy;
}
```

The RAPL (Running Average Power Limit) interface is used to monitor the power consumption of an Intel processor. In this code, we first initialize the RAPL library using `rapl_init()` (in the main function and parsing the error state using `raplError`). Then, we use the `rapl_get_power_limit()` and `rapl_get_energy()` functions to get the package power limit and energy consumption, respectively. Finally, we cleanup the RAPL library using `rapl_finish()` (either here or in the main function).

![Output]()

### Using NVML

```c++
if(!nvmlError){
    // Get the device handle for the first GPU on the system
    nvmlDevice_t device;
    result = nvmlDeviceGetHandleByIndex(0, &device);
    if (NVML_SUCCESS != result) {
        std::cout << "\n\n\t [ERROR] Failed to get handle for GPU 0: " <<  nvmlErrorString(result) << "\n";
        nvmlError = 1;
        nvmlShutdown();
    }
    else {
        // Get the total energy consumption of the GPU in millijoules
        unsigned long long energyConsumed;
        result = nvmlDeviceGetTotalEnergyConsumption(device, &energyConsumed);
        if (NVML_SUCCESS != result) {
            std::cout << "\n\n\t [ERROR] Failed to get total energy consumption of GPU 0: " << nvmlErrorString(result) << "\n";
            nvmlError = 1;
            nvmlShutdown();
        }
        else
            powerMetrics.gpuPower = result;
    }
}
```

In this code, we first initialize the NVML library using `nvmlInit()` (in the `main` function and passing the error state using `nvmlError`). Then, we get the handle of the first available device using `nvmlDeviceGetHandleByIndex()`. Next, we use the `nvmlDeviceGetPowerUsage()` function to get the power usage of the device in milliwatts. Finally, we cleanup the NVML library using `nvmlShutdown()` (either here or at the finish of this application).

![Output]()