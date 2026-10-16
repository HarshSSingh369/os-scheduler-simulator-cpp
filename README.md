# Process Scheduler Simulator

A C++ command-line simulator for comparing classic CPU scheduling policies.
Enter a small workload and inspect its Gantt-style execution timeline, per-
process completion data, waiting time, and turnaround time.

## Scheduling policies

- **FCFS:** first process to arrive runs first; non-preemptive.
- **SJF:** shortest available CPU burst runs next; non-preemptive.
- **SRTF:** the process with the shortest remaining time runs; preemptive.
- **Round Robin:** each ready process receives a fixed time quantum.
- **Compare all:** runs the same workload through all four policies.

Idle CPU time is shown explicitly. Average waiting and turnaround times are
reported for each run. The program simulates scheduling; it does not change
the operating system's real process priorities.

## Requirements

- C++17 compiler such as GCC or Clang.
- Terminal or command prompt.

## Compile and run

### Windows with GCC

```powershell
g++ -std=c++17 -Wall -Wextra -pedantic src/main.cpp -o scheduler.exe
.\scheduler.exe
```

### macOS or Linux

```sh
g++ -std=c++17 -Wall -Wextra -pedantic src/main.cpp -o scheduler
./scheduler
```

Enter each process's arrival time and CPU burst, choose a policy, and enter a
time quantum if you select Round Robin or Compare all.

## Metrics

- **Turnaround time** = completion time − arrival time.
- **Waiting time** = turnaround time − CPU burst.
- **Average waiting/turnaround** = arithmetic mean across all processes.

## Project structure

```text
os-scheduler-simulator-cpp/
├── src/main.cpp
└── README.md
```
