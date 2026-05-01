# You Spin Me Round Robin

Simulates a round-robin preemptive scheduler with a list of processes, their arrival time, and their job duration, along with a quantum size-- the atomic unit of time for assigning CPU time.

## Building

Run the Makefile to compile and build the executable `rr`.

```shell
make
```

## Running

Run the executable with the following args:
- `process_list.txt` text file with list of processes. First line is number of processes `n`, followed by `pid, arrival_time, burst_time` on the following N lines
- `quantum_length` how many units of time in a quantum-- a single slot of CPU time for a process to do work

```shell
./rr <process_list.txt> <quantum_length>
```

The result output to stdout shows:
- Average waiting time (time between arrival and completion while the process is NOT doing work)
- Average response time (time between arrival and first time doing work on CPU)
```shell
Average waiting time: XXX
Average response time: YYY
```

## Cleaning up

Use the Makefile to clean the project directory of object files / executables.

```shell
make clean
```
