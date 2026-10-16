#include <algorithm>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <queue>
#include <sstream>
#include <string>
#include <vector>

struct Process {
    std::string id;
    int arrival;
    int burst;
};

struct Segment {
    std::string process;
    int start;
    int end;
};

struct Result {
    std::string name;
    std::vector<Segment> timeline;
    std::vector<int> completion;
};

void addSegment(std::vector<Segment>& timeline, const std::string& process, int start, int end) {
    if (!timeline.empty() && timeline.back().process == process && timeline.back().end == start) {
        timeline.back().end = end;
    } else {
        timeline.push_back({process, start, end});
    }
}

Result runFcfs(const std::vector<Process>& processes) {
    Result result{"FCFS", {}, std::vector<int>(processes.size(), 0)};
    std::vector<std::size_t> order(processes.size());
    std::iota(order.begin(), order.end(), 0);
    std::stable_sort(order.begin(), order.end(), [&](std::size_t a, std::size_t b) {
        return processes[a].arrival < processes[b].arrival;
    });
    int time = 0;
    for (const auto i : order) {
        if (time < processes[i].arrival) {
            addSegment(result.timeline, "IDLE", time, processes[i].arrival);
            time = processes[i].arrival;
        }
        addSegment(result.timeline, processes[i].id, time, time + processes[i].burst);
        time += processes[i].burst;
        result.completion[i] = time;
    }
    return result;
}

Result runSjf(const std::vector<Process>& processes) {
    Result result{"SJF (non-preemptive)", {}, std::vector<int>(processes.size(), 0)};
    std::vector<bool> done(processes.size(), false);
    int completed = 0;
    int time = 0;
    while (completed < static_cast<int>(processes.size())) {
        int selected = -1;
        for (std::size_t i = 0; i < processes.size(); ++i) {
            if (done[i] || processes[i].arrival > time) continue;
            if (selected < 0 || processes[i].burst < processes[selected].burst ||
                (processes[i].burst == processes[selected].burst && processes[i].arrival < processes[selected].arrival)) {
                selected = static_cast<int>(i);
            }
        }
        if (selected < 0) {
            int nextArrival = 0;
            bool first = true;
            for (std::size_t i = 0; i < processes.size(); ++i) {
                if (!done[i] && (first || processes[i].arrival < nextArrival)) {
                    nextArrival = processes[i].arrival;
                    first = false;
                }
            }
            addSegment(result.timeline, "IDLE", time, nextArrival);
            time = nextArrival;
            continue;
        }
        const auto& process = processes[selected];
        addSegment(result.timeline, process.id, time, time + process.burst);
        time += process.burst;
        result.completion[selected] = time;
        done[selected] = true;
        ++completed;
    }
    return result;
}

Result runSrtf(const std::vector<Process>& processes) {
    Result result{"SRTF (preemptive)", {}, std::vector<int>(processes.size(), 0)};
    std::vector<int> remaining;
    remaining.reserve(processes.size());
    for (const auto& process : processes) remaining.push_back(process.burst);
    int completed = 0;
    int time = 0;
    while (completed < static_cast<int>(processes.size())) {
        int selected = -1;
        for (std::size_t i = 0; i < processes.size(); ++i) {
            if (remaining[i] <= 0 || processes[i].arrival > time) continue;
            if (selected < 0 || remaining[i] < remaining[selected] ||
                (remaining[i] == remaining[selected] && processes[i].arrival < processes[selected].arrival)) {
                selected = static_cast<int>(i);
            }
        }
        if (selected < 0) {
            addSegment(result.timeline, "IDLE", time, time + 1);
            ++time;
            continue;
        }
        addSegment(result.timeline, processes[selected].id, time, time + 1);
        --remaining[selected];
        ++time;
        if (remaining[selected] == 0) {
            result.completion[selected] = time;
            ++completed;
        }
    }
    return result;
}

Result runRoundRobin(const std::vector<Process>& processes, int quantum) {
    Result result{"Round Robin (q=" + std::to_string(quantum) + ")", {}, std::vector<int>(processes.size(), 0)};
    std::vector<int> remaining;
    remaining.reserve(processes.size());
    for (const auto& process : processes) remaining.push_back(process.burst);
    std::vector<std::size_t> order(processes.size());
    std::iota(order.begin(), order.end(), 0);
    std::stable_sort(order.begin(), order.end(), [&](std::size_t a, std::size_t b) {
        return processes[a].arrival < processes[b].arrival;
    });
    std::queue<std::size_t> ready;
    std::size_t nextArrival = 0;
    int completed = 0;
    int time = 0;

    while (completed < static_cast<int>(processes.size())) {
        if (ready.empty() && nextArrival < order.size() && time < processes[order[nextArrival]].arrival) {
            const int arrival = processes[order[nextArrival]].arrival;
            addSegment(result.timeline, "IDLE", time, arrival);
            time = arrival;
        }
        while (nextArrival < order.size() && processes[order[nextArrival]].arrival <= time) {
            ready.push(order[nextArrival++]);
        }
        if (ready.empty()) continue;

        const auto index = ready.front();
        ready.pop();
        const int duration = std::min(quantum, remaining[index]);
        addSegment(result.timeline, processes[index].id, time, time + duration);
        time += duration;
        remaining[index] -= duration;
        while (nextArrival < order.size() && processes[order[nextArrival]].arrival <= time) {
            ready.push(order[nextArrival++]);
        }
        if (remaining[index] == 0) {
            result.completion[index] = time;
            ++completed;
        } else {
            ready.push(index);
        }
    }
    return result;
}

double averageWaiting(const std::vector<Process>& processes, const Result& result) {
    double sum = 0;
    for (std::size_t i = 0; i < processes.size(); ++i) {
        sum += result.completion[i] - processes[i].arrival - processes[i].burst;
    }
    return sum / processes.size();
}

double averageTurnaround(const std::vector<Process>& processes, const Result& result) {
    double sum = 0;
    for (std::size_t i = 0; i < processes.size(); ++i) sum += result.completion[i] - processes[i].arrival;
    return sum / processes.size();
}

void printResult(const std::vector<Process>& processes, const Result& result) {
    std::cout << "\n" << result.name << "\n";
    if (result.timeline.empty()) return;
    std::cout << "Gantt: " << result.timeline.front().start;
    for (const auto& segment : result.timeline) {
        std::cout << " --" << segment.process << "--> " << segment.end;
    }
    std::cout << "\n\n" << std::left << std::setw(10) << "Process" << std::right
              << std::setw(10) << "Arrival" << std::setw(10) << "Burst"
              << std::setw(12) << "Complete" << std::setw(12) << "Turnaround"
              << std::setw(10) << "Waiting" << "\n";
    for (std::size_t i = 0; i < processes.size(); ++i) {
        const int turnaround = result.completion[i] - processes[i].arrival;
        const int waiting = turnaround - processes[i].burst;
        std::cout << std::left << std::setw(10) << processes[i].id << std::right
                  << std::setw(10) << processes[i].arrival << std::setw(10) << processes[i].burst
                  << std::setw(12) << result.completion[i] << std::setw(12) << turnaround
                  << std::setw(10) << waiting << "\n";
    }
    std::cout << std::fixed << std::setprecision(2)
              << "Average waiting: " << averageWaiting(processes, result)
              << "   Average turnaround: " << averageTurnaround(processes, result) << "\n";
}

int readInt(const std::string& prompt, int minimum, int maximum) {
    for (;;) {
        std::cout << prompt;
        std::string line;
        if (!std::getline(std::cin, line)) return minimum;
        std::stringstream stream(line);
        int value;
        char extra;
        if ((stream >> value) && !(stream >> extra) && value >= minimum && value <= maximum) return value;
        std::cout << "Enter a whole number from " << minimum << " to " << maximum << ".\n";
    }
}

int main() {
    std::cout << "PROCESS SCHEDULER SIMULATOR\n"
              << "Compare scheduling policies on a small process workload.\n\n";
    const int count = readInt("Number of processes (1-12): ", 1, 12);
    std::vector<Process> processes;
    for (int i = 0; i < count; ++i) {
        std::cout << "\nProcess P" << i + 1 << "\n";
        const int arrival = readInt("  Arrival time (0-100): ", 0, 100);
        const int burst = readInt("  CPU burst (1-100): ", 1, 100);
        processes.push_back({"P" + std::to_string(i + 1), arrival, burst});
    }

    std::cout << "\n1. FCFS  2. SJF  3. SRTF  4. Round Robin  5. Compare all\n";
    const int choice = readInt("Choose policy: ", 1, 5);
    std::vector<Result> results;
    if (choice == 1 || choice == 5) results.push_back(runFcfs(processes));
    if (choice == 2 || choice == 5) results.push_back(runSjf(processes));
    if (choice == 3 || choice == 5) results.push_back(runSrtf(processes));
    if (choice == 4 || choice == 5) {
        const int quantum = readInt("Time quantum (1-100): ", 1, 100);
        results.push_back(runRoundRobin(processes, quantum));
    }

    for (const auto& result : results) printResult(processes, result);
    if (results.size() > 1) {
        std::cout << "\nCOMPARISON (lower average is better)\n"
                  << std::left << std::setw(25) << "Policy" << std::right
                  << std::setw(18) << "Avg waiting" << std::setw(20) << "Avg turnaround" << "\n";
        for (const auto& result : results) {
            std::cout << std::left << std::setw(25) << result.name << std::right
                      << std::setw(18) << std::fixed << std::setprecision(2) << averageWaiting(processes, result)
                      << std::setw(20) << averageTurnaround(processes, result) << "\n";
        }
    }
    return 0;
}
