# Description of Selected Metrics

The table presented below does not agree 1:1 with the structures created as part of the implementation of the application itself. For example - we decided to separately group all information on energy consumption by a given node.

| Metric Name / Metric Groups | 	Unit	| Description/Concerns |
| --- | --- | --- |
| **System Metrics** | 		
|system.processes.running|number of processes|Number of processes in the R state|
|system.processes.blocked|number of processes|Number of processes waiting for I/O operations to complete|
|system.context.switch.rate|number of context switches per second|Number of context switches per unit time|
|system.interrupt.rate|number of interrupts per second|Number of all interrupts serviced in unit time|
| **Processor Metrics** |		
|processor.time.user|%|Time in user mode|
|processor.time.nice|%|Time in user mode with low priority|
|processor.time.system|%|Time in system mode|
|processor.time.idle|%|Time in idle task mode|
|processor.time.iowait|%|Time waiting for I/O operations to complete|
|processor.time.irq|%|Time spent handling interrupts|
|processor.time.softirq|%|Time spent handling softirqs|
|processor.time.steal|%|Time in other SO in virtualization mode|
|processor.time.guest|%|Time running virtual CPU for other SOs under kernel control|
|processor.time.guest.nice|%|Time running virtual CPU with low priority for other SOs under kernel control|
|processor.instructions.retired.rate|number of instructions per second|Actually executed instructions|
|processor.cycles.rate|number of cycles per second|Clock cycles during core operation (possible Turbo Boost)|
|processor.cycles.reference.rate|number of cycles per second|Reference clock cycles|
|processor.frequency.relative|%|Average core clock frequency, also taking into account Turbo Boost|
|processor.frequence.active.relative|%|Average core clock frequency when not in C0 state, also taking into account Turbo Boost|
|processor.cache.l2.hit.rate|hits per second|Number of hits in L2 cache|
|processor.cache.l2.miss.rate|misses per second|Number of misses in L2 cache|
|processor.cache.l3.hit.rate|hits per second|Number of hits in L3 cache|
|processor.cache.l3.hit.snoop.rate|hits per second|Number of hits in L3 cache, with sibling L2 memory references|
|processor.cache.l3.miss.rate|misses per second|Number of misses in L3 cache|
|processor.power|W|Power consumed by the processor|
|**I/O Metrics**|		
|storage.read.rate|MB/s|Read data|
|storage.read.time|s|Time to read data|
|storage.read.operations.rate|number of operations per second|Read operations|
|storage.write.rate|MB/s|Written data|
|storage.write.time|s|Time to write data|
|storage.write.operations.rate|number of operations per second|Write operations|
|storage.flush.time|s|Time to execute flush operation|
|storage.flush.operations.rate|number of operations per second|Flush operations|
|**Memory Metrics**|		
|memory.used|MB|Used RAM|
|memory.cached|MB|Cache for files read from disk|
|swap.used|MB|Used swap|
|swap.cached|MB|Data previously written from memory to disk, retrieved and still in swap file|
|memory.active|MB|Data used in recent period|
|memory.inactive|MB|Data used prior to memory.active|
|memory.page.in.rate|number of pages per second|Pages read|
|memory.page.out.rate|number of pages per second|Pages written|
|memory.page.fault.rate|number of pages per second|Page faults|
|memory.page.faults.major.rate|nNumber of pages per second|Page faults (require reading from disk)|
|memory.page.free.rate|number of pages per second|Freeing pages|
|memory.page.activate.rate|number of pages per second|Page activation|
|memory.page.deactivate.rate|number of pages per second|Page deactivation|
|memory.read.rate|MB/s|Read from memory|
|memory.write.rate|MB/s|Write to memory|
|memory.io.rate|MB/s|Read/Write requests to/from all I/O devices|
|memory.power|W|Power consumed by memory|
|**Network Metrics**|		
|network.receive.rate|MB/s|Received data|
|network.receive.packets.rate|number of packets per second|Received packets|
|network.send.rate|MB/s|Sent data|
|network.send.packets.rate|number of packets per second|Sent packets|