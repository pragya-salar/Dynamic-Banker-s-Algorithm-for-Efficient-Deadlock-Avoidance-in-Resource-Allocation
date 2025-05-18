#include <iostream>
#include <vector>
#include <algorithm>
#include <climits>

using namespace std;

struct Process {
    int id;
    int arrivalTime;
    int burstTime;
    int remainingTime;
    vector<int> maximum;
    vector<int> allocation;
    vector<int> need;
    bool completed;
    bool removed;
    int priority; // Lower number means higher priority
};

bool compareNeed(const Process& a, const Process& b) {
    int maxNeedA = *max_element(a.need.begin(), a.need.end());
    int maxNeedB = *max_element(b.need.begin(), b.need.end());
    return maxNeedA < maxNeedB;
}

bool requestResources(int processId, vector<int>& request, vector<Process>& processes, vector<int>& available) {
    Process& p = processes[processId];
    int d = available.size();

    for (int i = 0; i < d; i++) {
        if (request[i] > p.need[i]) {
            cout << "\nRequest exceeds the process's declared maximum need. Denied.\n";
            return false;
        }
    }

    for (int i = 0; i < d; i++) {
        if (request[i] > available[i]) {
            cout << "\n Not enough available resources. Request must wait.\n";
            return false;
        }
    }

    for (int i = 0; i < d; i++) {
        available[i] -= request[i];
        p.allocation[i] += request[i];
        p.need[i] -= request[i];
    }

    vector<int> work = available;
    vector<bool> finish(processes.size(), false);

    while (true) {
        bool progress = false;
        for (int i = 0; i < processes.size(); i++) {
            if (!finish[i] && !processes[i].removed && !processes[i].completed) {
                bool canFinish = true;
                for (int j = 0; j < d; j++) {
                    if (processes[i].need[j] > work[j]) {
                        canFinish = false;
                        break;
                    }
                }
                if (canFinish) {
                    for (int j = 0; j < d; j++) {
                        work[j] += processes[i].allocation[j];
                    }
                    finish[i] = true;
                    progress = true;
                }
            }
        }
        if (!progress) break;
    }

    for (bool done : finish) {
        if (!done) {
            for (int i = 0; i < d; i++) {
                available[i] += request[i];
                p.allocation[i] -= request[i];
                p.need[i] += request[i];
            }
            cout << "\n System would be unsafe. Request denied.\n";
            return false;
        }
    }

    cout << "\n Request granted. System remains in a safe state.\n";
    return true;
}

int main() {
    int d;
    cout << "Enter the number of resources: ";
    cin >> d;

    vector<int> available(d);
    cout << "Enter available resources: ";
    for (int i = 0; i < d; i++) {
        cin >> available[i];
    }

    int n;
    cout << "Enter the number of processes: ";
    cin >> n;

    vector<Process> processes;
    int processIdCounter = 0;

    for (int i = 0; i < n; i++) {
        Process p;
        p.id = processIdCounter++;
        p.completed = false;
        p.removed = false;

        cout << "Enter arrival time and burst time for process " << p.id << ": ";
        cin >> p.arrivalTime >> p.burstTime;
        p.remainingTime = p.burstTime;

        cout << "Enter priority (lower = higher): ";
        cin >> p.priority;

        p.maximum.resize(d);
        p.allocation.resize(d);
        p.need.resize(d);

        cout << "Enter maximum resources: ";
        for (int j = 0; j < d; j++) cin >> p.maximum[j];

        cout << "Enter allocation: ";
        for (int j = 0; j < d; j++) {
            cin >> p.allocation[j];
            available[j] -= p.allocation[j];
            p.need[j] = p.maximum[j] - p.allocation[j];
        }

        processes.push_back(p);
    }

    vector<Process> safeSequence;
    int currentTime = 0;

    while (true) {
        bool allCompleted = true;
        for (const auto& p : processes) {
            if (!p.completed && !p.removed) {
                allCompleted = false;
                break;
            }
        }
        if (allCompleted) break;

        bool progressMade = false;

        char removalChoice;
        cout << "\nDo you want to remove a process? (y/n): ";
        cin >> removalChoice;
        if (removalChoice == 'y' || removalChoice == 'Y') {
            int removeId;
            cout << "Enter Process ID to remove: ";
            cin >> removeId;
            if (removeId >= 0 && removeId < processIdCounter && !processes[removeId].completed && !processes[removeId].removed) {
                for (int j = 0; j < d; j++) {
                    available[j] += processes[removeId].allocation[j];
                }
                processes[removeId].removed = true;
                progressMade = true;
                cout << "Process P" << removeId << " removed.\n";
            } else {
                cout << "Invalid or already completed/removed.\n";
            }
        }

        vector<Process*> readyQueue;
        for (auto& p : processes) {
            if (!p.completed && !p.removed && p.arrivalTime <= currentTime) {
                bool canProceed = true;
                for (int j = 0; j < d; j++) {
                    if (p.need[j] > available[j]) {
                        canProceed = false;
                        break;
                    }
                }
                if (canProceed) readyQueue.push_back(&p);
            }
        }

        sort(readyQueue.begin(), readyQueue.end(), [](Process* a, Process* b) {
            return a->priority == b->priority ? compareNeed(*a, *b) : a->priority < b->priority;
        });

        if (!readyQueue.empty()) {
            Process* current = readyQueue[0];
            cout << "\nProcess P" << current->id << " executed and completed.\n";
            for (int j = 0; j < d; j++) available[j] += current->allocation[j];
            current->completed = true;
            safeSequence.push_back(*current);
            progressMade = true;
        } else {
            cout << "\nNo process can proceed at time " << currentTime << ". System may be in deadlock.\n";
        }

        cout << "\n1. Add new process\n2. Request resources\n3. Continue\nEnter choice: ";
        int choice;
        cin >> choice;

        if (choice == 1) {
            Process newP;
            newP.id = processIdCounter++;
            newP.completed = false;
            newP.removed = false;

            cout << "Enter arrival time and burst time: ";
            cin >> newP.arrivalTime >> newP.burstTime;
            newP.remainingTime = newP.burstTime;

            cout << "Enter priority: ";
            cin >> newP.priority;

            newP.maximum.resize(d);
            newP.allocation.resize(d);
            newP.need.resize(d);

            cout << "Enter maximum resources: ";
            for (int j = 0; j < d; j++) cin >> newP.maximum[j];

            cout << "Enter allocation: ";
            for (int j = 0; j < d; j++) {
                cin >> newP.allocation[j];
                available[j] -= newP.allocation[j];
                newP.need[j] = newP.maximum[j] - newP.allocation[j];
            }

            processes.push_back(newP);
            progressMade = true;
        } else if (choice == 2) {
            int pid;
            cout << "Enter process ID to request resources: ";
            cin >> pid;
            if (pid >= 0 && pid < processes.size() && !processes[pid].completed && !processes[pid].removed) {
                vector<int> req(d);
                cout << "Enter request vector: ";
                for (int i = 0; i < d; i++) cin >> req[i];
                requestResources(pid, req, processes, available);
            } else {
                cout << "Invalid process ID.\n";
            }
        }

        if (!progressMade) {
            cout << "\nNo progress can be made. System may be in deadlock.\n";
            break;
        }

        currentTime++;
    }

    cout << "\nFinal Safe Sequence: ";
    for (const auto& p : safeSequence) cout << "P" << p.id << " ";
    cout << endl;

    return 0;
}