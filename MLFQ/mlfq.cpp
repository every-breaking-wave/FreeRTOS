#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

using namespace std;

class Process {
public:
    string name;
    int arrival_time;
    int duration;
    bool started;

    Process(string n, int arrival, int burst)
        : name(n), arrival_time(arrival), duration(burst), started(false) {}
};

// Define time slice for each queue
vector<int> time_slices = { 1, 2, 4, 8 };  // Time slices for each queue (higher priority gets smaller time slice)

void log_queue(const vector<vector<Process>>& queue, int current_time) {
    cout << "[current time: " << current_time << "]\n";
    for (size_t i = 0; i < queue.size(); ++i) {
        cout << "Queue " << i << ": ";
        for (const auto& p : queue[i]) {
            cout << "[name: " << p.name << ", duration: " << p.duration << "] ";
        }
        cout << endl;
    }
    cout << endl;
}

int mlfq_schedule(vector<vector<Process>>& queue, vector<Process>& completed_processes, int current_time, int reset_interval) {
    // Select a process from the queue to execute
    for (size_t i = 0; i < queue.size(); ++i) {
        if (!queue[i].empty()) {
            Process process = queue[i].front();
            queue[i].erase(queue[i].begin());
            
            // Execute the process for the time slice based on the priority (queue index)
            int time_slice = time_slices[i];
            int time_to_run = min(time_slice, process.duration);

            if (process.duration <= time_to_run) {
                // Process completed
                current_time += process.duration;
                process.duration = 0;
                completed_processes.push_back(process);
            } else {
                // Process executes for the time slice, then move to the next queue
                current_time += time_to_run;
                process.duration -= time_to_run;

                // Move the process to the next priority queue (lower priority)
                if (i + 1 == queue.size()) {
                    queue[i].push_back(process);  // Stay in the current queue if it's the lowest
                } else {
                    queue[i + 1].push_back(process);  // Move to the next queue
                }
            }

            // Re-insert all processes to the highest priority queue at the specified interval
            if (current_time % reset_interval == 0) {
                for (size_t j = 1; j < queue.size(); ++j) {
                    for (auto& p : queue[j]) {
                        queue[0].push_back(p);
                    }
                    queue[j].clear();
                }
                cout << "Move all processes to the highest priority queue at time " << current_time << "\n";
            }

            break;
        }
    }

    return current_time;
}

vector<Process> run(vector<Process>& processes, int queue_num = 4, int reset_interval = 10) {
    vector<vector<Process>> queue(queue_num);
    int current_time = 0;
    vector<Process> completed_processes;

    while (true) {
        for (auto& process : processes) {
            if (!process.started && process.arrival_time <= current_time) {
                queue[0].push_back(process);  // Add to highest priority queue
                process.started = true;
            }
        }

        log_queue(queue, current_time);

        current_time = mlfq_schedule(queue, completed_processes, current_time, reset_interval);

        // Check if all processes have completed
        if (all_of(processes.begin(), processes.end(), [](const Process& p) { return p.duration == 0; })) {
            break;
        }
    }

    return completed_processes;
}

int main() {
    vector<Process> processes = {
        Process("P1", 0, 3),
        Process("P2", 1, 6),
        Process("P3", 3, 2),
        Process("P4", 5, 7),
        Process("P5", 7, 4),
        Process("P6", 8, 1)     
    };
    vector<Process> completed_processes = run(processes);

    for (const auto& process : completed_processes) {
        cout << "Process " << process.name << " completed\n";
    }

    return 0;
}
