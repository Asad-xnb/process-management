#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <fstream>
#include <cstdio>
#include <dirent.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <cstring>
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <algorithm>

using namespace std;

string CURRENT_DIR = ".";

vector<string> split(const string& str) {
    vector<string> tokens;
    istringstream iss(str);
    string token;
    while (iss >> token)
        tokens.push_back(token);
    return tokens;
}

void createFile(const string& filename) {
    string filepath = CURRENT_DIR + "/" + filename;
    ofstream ofs(filepath);
    if (ofs) {
        cout << "File '" << filename << "' created successfully.\n";
    } else {
        cout << "Failed to create file.\n";
    }
}

void readFile(const string& filename) {
    string filepath = CURRENT_DIR + "/" + filename;
    ifstream ifs(filepath);
    if (!ifs) {
        cout << "File not found.\n";
        return;
    }
    cout << "---- Content of '" << filename << "' ----\n";
    string line;
    while (getline(ifs, line))
        cout << line << '\n';
    cout << "---- End of file ----\n";
}

void deleteFile(const string& filename) {
    string filepath = CURRENT_DIR + "/" + filename;
    if (remove(filepath.c_str()) == 0) {
        cout << "File '" << filename << "' deleted successfully.\n";
    } else {
        cout << "Failed to delete file.\n";
    }
}

void listFiles() {
    DIR* dir = opendir(CURRENT_DIR.c_str());
    if (!dir) {
        cout << "Directory '" << CURRENT_DIR << "' does not exist or cannot be opened.\n";
        return;
    }
    cout << "Files in '" << CURRENT_DIR << "':\n";
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        string name = entry->d_name;
        if (name != "." && name != "..")
            cout << "  " << name << '\n';
    }
    closedir(dir);
}

void forkProcess() {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
    } else if (pid == 0) {
        cout << "[Child] PID: " << getpid() << ", PPID: " << getppid() << "\n";
        cout << "[Child] Exiting...\n";
        exit(0);
    } else {
        cout << "[Parent] Created child process with PID: " << pid << "\n";
    }
}

void showPid() {
    cout << "Current PID: " << getpid() << "\n";
}

void showPpid() {
    cout << "Parent PID: " << getppid() << "\n";
}

void waitForChild() {
    cout << "Waiting for child process to finish...\n";
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return;
    } else if (pid == 0) {
        cout << "[Child] PID: " << getpid() << " started, sleeping 2 seconds...\n";
        sleep(2);
        cout << "[Child] PID: " << getpid() << " exiting.\n";
        exit(42);
    } else {
        int status;
        pid_t wpid = waitpid(pid, &status, 0);
        if (wpid == -1) {
            perror("waitpid failed");
            return;
        }
        if (WIFEXITED(status)) {
            cout << "[Parent] Child " << wpid << " exited with status " << WEXITSTATUS(status) << ".\n";
        } else {
            cout << "[Parent] Child ended abnormally.\n";
        }
    }
}

void listProcesses() {
    cout << "Listing processes from /proc (limited)...\n";
    DIR* dir = opendir("/proc");
    if (!dir) {
        perror("opendir /proc failed");
        return;
    }
    struct dirent* entry;
    vector<int> pids;
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_type == DT_DIR) {
            string name = entry->d_name;
            if (all_of(name.begin(), name.end(), ::isdigit)) {
                pids.push_back(stoi(name));
            }
        }
    }
    closedir(dir);
    cout << "PID\tCMD\n";
    for (int pid : pids) {
        string cmdPath = "/proc/" + to_string(pid) + "/comm";
        ifstream ifs(cmdPath);
        string cmd;
        if (ifs.is_open()) {
            getline(ifs, cmd);
            cout << pid << "\t" << cmd << '\n';
        }
    }
}

void killProcess(const string& pidStr) {
    pid_t pid = stoi(pidStr);
    if (kill(pid, SIGTERM) == 0) {
        cout << "Sent SIGTERM to process " << pid << ".\n";
    } else {
        perror("Failed to kill process");
    }
}

void listDirContents() {
    cout << "Current directory contents:\n";
    DIR* dir = opendir(".");
    if (!dir) {
        perror("opendir failed");
        return;
    }
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        cout << "  " << entry->d_name << '\n';
    }
    closedir(dir);
}

// ------------------- FCFS Scheduling Simulation -------------------

struct Process {
    int pid;
    int burstTime;
    int waitingTime;
    int turnaroundTime;
};

void saveGanttChart(const vector<Process>& processes) {
    string filename = CURRENT_DIR + "/fcfs_gantt.txt";
    ofstream ofs(filename);
    if (ofs) {
        ofs << "Gantt Chart:\n";
        ofs << "0";
        for (const auto& p : processes) {
            ofs << " -- P" << p.pid << " -- " << p.turnaroundTime;
        }
        ofs << "\n";
        cout << "\nGantt chart saved to '" << filename << "'\n";
    } else {
        cout << "Failed to save Gantt chart file.\n";
    }
}

void fcfsSimulation() {
    int n;
    cout << "Enter number of processes: ";
    cin >> n;
    vector<Process> processes(n);
    for (int i = 0; i < n; i++) {
        processes[i].pid = i + 1;
        cout << "Enter burst time for process " << processes[i].pid << ": ";
        cin >> processes[i].burstTime;
    }

    processes[0].waitingTime = 0;
    processes[0].turnaroundTime = processes[0].burstTime;

    for (int i = 1; i < n; i++) {
        processes[i].waitingTime = processes[i-1].waitingTime + processes[i-1].burstTime;
        processes[i].turnaroundTime = processes[i].waitingTime + processes[i].burstTime;
    }

    cout << "\nPID\tBurst Time\tWaiting Time\tTurnaround Time\n";
    for (const auto& p : processes) {
        cout << p.pid << "\t" << p.burstTime << "\t\t" << p.waitingTime << "\t\t" << p.turnaroundTime << "\n";
    }
    
    cout << endl;
    cout << endl;

    cout << "Do you want to save the Gantt chart? (y/n): ";
    char saveGantt;
    cin >> saveGantt;
    if (saveGantt == 'y' || saveGantt == 'Y') {
        saveGanttChart(processes);
    } else {
        cout << "Gantt chart not saved.\n";
    }

    
}

void fileMenu() {
    cout << "\n--- File Management Menu ---\n";
    cout << "Commands: create <filename>\n read <filename>\n delete <filename>\n list\n back\n";
    string line;
    while (true) {
        cout << "file> ";
        getline(cin, line);
        auto tokens = split(line);
        if (tokens.empty()) continue;
        string cmd = tokens[0];

        if (cmd == "create" && tokens.size() == 2) {
            createFile(tokens[1]);
        } else if (cmd == "read" && tokens.size() == 2) {
            readFile(tokens[1]);
        } else if (cmd == "delete" && tokens.size() == 2) {
            deleteFile(tokens[1]);
        } else if (cmd == "list" && tokens.size() == 1) {
            listFiles();
        } else if (cmd == "back") {
            cout << "Returning to main menu...\n";
            break;
        } else {
            cout << "Invalid file command. Try again.\n";
        }
    }
}

void processMenu() {
    cout << "\n--- Process Management Menu ---\n";
    cout << "Commands: fork\n pid\n ppid\n wait\n ps\n kill <pid>\n dir\n back\n";
    string line;
    while (true) {
        cout << "process> ";
        getline(cin, line);
        auto tokens = split(line);
        if (tokens.empty()) continue;
        string cmd = tokens[0];

        if (cmd == "fork" && tokens.size() == 1) {
            forkProcess();
        } else if (cmd == "pid" && tokens.size() == 1) {
            showPid();
        } else if (cmd == "ppid" && tokens.size() == 1) {
            showPpid();
        } else if (cmd == "wait" && tokens.size() == 1) {
            waitForChild();
        } else if (cmd == "ps" && tokens.size() == 1) {
            listProcesses();
        } else if (cmd == "kill" && tokens.size() == 2) {
            killProcess(tokens[1]);
        } else if (cmd == "dir" && tokens.size() == 1) {
            listDirContents();
        } else if (cmd == "back") {
            cout << "Returning to main menu...\n";
            break;
        } else {
            cout << "Invalid process command. Try again.\n";
        }
    }
}

void helpMenu() {
    cout << "\nAvailable Commands:\n";
    cout << "  file          - Enter file management menu\n";
    cout << "  process       - Enter process management menu\n";
    cout << "  dir           - Directory Related Commands\n";
    cout << "  help          - Show this help menu\n";
    cout << "  clear         - Clear the screen\n";
    cout << "  exit          - Exit the shell\n";
}

void makeDirectory(const string& dirname) {
    string path = CURRENT_DIR + "/" + dirname;
    if (mkdir(path.c_str(), 0700) == 0) {
        cout << "Directory '" << dirname << "' created successfully.\n";
    } else {
        perror("mkdir failed");
    }
    return;
}

void changeDirectory(const string& path) {
    if (chdir(path.c_str()) == 0) {
        char buffer[1024];
        getcwd(buffer, sizeof(buffer));
        CURRENT_DIR = buffer;
        cout << "Current directory changed to: " << CURRENT_DIR << '\n';
    } else {
        perror("cd failed");
    }
}

void removeDirectory(const string& dirname) {
    string path = CURRENT_DIR + "/" + dirname;
    if (rmdir(path.c_str()) == 0) {
        cout << "Directory '" << dirname << "' removed successfully.\n";
    } else {
        perror("rmdir failed");
    }
}

void printWorkingDirectory() {
    cout << "Current directory: " << system("pwd") << "\n\n";
}

void directoryMenu() {
    cout << "\n--- Directory Management Menu ---\n";
    cout << "\nAvailable Commands:\n";
    cout << "  make <directory>     - Enter file management menu\n";
    cout << "  cd <path>            - Enter process management menu\n";
    cout << "  rmdir <directory>    - Directory Related Commands\n";
    cout << "  ls                   - List files in the current directory\n";
    cout << "  pwd                  - Print current working directory\n";
    
    cout << "  help                 - Show this help menu\n";
    cout << "  clear                - Clear the screen\n";
    cout << "  back                 - Get back to Main Menu\n";
    cout << "  exit                 - Exit the shell\n";
    string line;
    while (true) {
        cout << "Directory> ";
        getline(cin, line);
        auto tokens = split(line);
        if (tokens.empty()) continue;
        string cmd = tokens[0];

        if (cmd == "make" && tokens.size() == 2) {
            cout << "\nPlease provide a directory name.\n";
            makeDirectory(tokens[1]);

        } 
        

        else if (cmd == "back") {
            cout << "Returning to main menu...\n";
            break;
        } else {
            cout << "Invalid process command. Try again.\n";
        }
    }
}

// void directoryMenu() {
//     cout << "\nAvailable Commands:\n";
//     cout << "  make <directory>     - Enter file management menu\n";
//     cout << "  cd <path>            - Enter process management menu\n";
//     cout << "  rmdir <directory>    - Directory Related Commands\n";
//     cout << "  ls                   - List files in the current directory\n";
//     cout << "  pwd                  - Print current working directory\n";
    
//     cout << "  help                 - Show this help menu\n";
//     cout << "  clear                - Clear the screen\n";
//     cout << "  back                 - Get back to Main Menu\n";
//     cout << "  exit                 - Exit the shell\n";
// }

void ensureFilesDir() {
    struct stat st = {0};
    if (stat(CURRENT_DIR.c_str(), &st) == -1) {
        mkdir(CURRENT_DIR.c_str(), 0700);
    }
}

int main() {
    ensureFilesDir();

    cout << "Welcome to Ubuntu Shell-like Process Manager!\n";
    cout << "Type 'help' to see available commands.\n";

    string line;
    while (true) {
        cout << "> ";
        getline(cin, line);
        auto tokens = split(line);
        if (tokens.empty()) continue;

        string cmd = tokens[0];

        if (cmd == "file") {
            fileMenu();
        } else if (cmd == "process") {
            processMenu();
        } else if (cmd == "dir") {
            directoryMenu();
        } else if (cmd == "help") {
            helpMenu();
        } else if (cmd == "clear") {
            system("clear");
        } else if (cmd == "exit") {
            cout << "Exiting shell. Goodbye!\n";
            break;
        } else {
            cout << "Unknown command. Type 'help' for list of commands.\n";
        }
    }

    return 0;
}
