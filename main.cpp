#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <fstream>
#include <cstdio>
#include <dirent.h>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;
using namespace std;

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"

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
    cout << "---- '" << filename << "' ----\n\n";
    string line;
    while (getline(ifs, line))
        cout << line << '\n';
    cout << "\n---- End ----\n";
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
    cout << "Files in '" << CURRENT_DIR << "/" <<"':\n";
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        string name = entry->d_name;
        if (name != "." && name != "..")
            cout << "  " << name << '\n';
    }
    closedir(dir);
}

void forkProcess(const string& programPath) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return;
    }

    if (pid == 0) {
        string fullPath = CURRENT_DIR + "/" + programPath;

        const char* cmd[] = {
            "gnome-terminal",
            "--",
            fullPath.c_str(),
            nullptr
        };

        execvp("gnome-terminal", (char* const*)cmd);
        perror("execvp failed");
        exit(1);
    } else {
        cout << "[Parent] Launched new terminal with child PID: " << pid << "\n";
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
    cout << "Launching htop...\n";
    int ret = system("htop");

    if (ret == -1) {
        perror("Failed to launch htop");
    } else {
        cout << "\nReturned from htop.\n";
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

    try {
        if (!fs::exists(path)) {
            cout << "Directory '" << dirname << "' does not exist.\n";
            return;
        }

        if (!fs::is_directory(path)) {
            cout << "'" << dirname << "' is not a directory.\n";
            return;
        }

        if (fs::is_empty(path)) {
            fs::remove(path);
            cout << "Directory '" << dirname << "' removed successfully.\n";
        } else {
            cout << "Directory '" << dirname << "' is not empty. Do you want to delete it and all its contents? (y/n): ";
            char choice;
            cin >> choice;
            cin.ignore();

            if (tolower(choice) == 'y') {
                fs::remove_all(path);
                cout << "Directory '" << dirname << "' and all its contents removed successfully.\n";
            } else {
                cout << "Operation cancelled. Directory not removed.\n";
            }
        }
    } catch (const fs::filesystem_error& e) {
        cerr << "Error: " << e.what() << "\n";
    }
}

void printWorkingDirectory() {
    cout << "Current working directory: \n";
    system("pwd");
    cout << endl;
}

void fMenu() {
    cout << "\n--- File Management Menu ---\n";

    cout << "\nAvailable Commands:\n";
    cout << "  create <filename>    - Create File\n";
    cout << "  read <filename>      - Read File\n";
    cout << "  delete <filename>    - Delete File\n";
    cout << "  list                 - List files in the current directory\n";
    
    cout << endl;
    cout << "  cd <path>            - Change current directory\n";
    cout << "  pwd                  - Print current working directory\n";

    cout << endl;
    cout << "\n\n";
    cout << "  help                 - Show this help menu\n";
    cout << "  clear                - Clear the screen\n";
    cout << "  back                 - Get back to Main Menu\n";
    cout << "  exit                 - Exit the shell\n";
}

void fileMenu() {
    system("clear");
    fMenu();
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
            system("clear");
            listFiles();
        } else if (cmd == "cd" && tokens.size() == 2) {
            system("clear");
            changeDirectory(tokens[1]);
        } else if (cmd == "pwd" && tokens.size() == 1) {
            system("clear");
            printWorkingDirectory();
        } 
        else if (cmd == "help" && tokens.size() == 1) {
            system("clear");
            fMenu();
        } else if (cmd == "clear" && tokens.size() == 1) {
            system("clear");
        }
        else if (cmd == "back") {
            system("clear");
            cout << "Returning to main menu...\n";
            break;
        } else if (cmd == "exit") {
            system("clear");
            cout << "Exiting ...\n\n";
            exit(0);
        }
        else {
            cout << "Invalid directory command. Try again.\n";
        }
    }
}

void dMenu() {
    cout << "\n--- Directory Management Menu ---\n";
    cout << "\nAvailable Commands:\n";
    cout << "  make <directory>     - make directory\n";
    cout << "  cd <path>            - change directory\n";
    cout << "  remove <directory>   - remove directory\n";
    cout << "  ls                   - List files in the current directory\n";
    cout << "  pwd                  - Print current working directory\n";
    
    cout << "\n\n";
    cout << "  help                 - Show this help menu\n";
    cout << "  clear                - Clear the screen\n";
    cout << "  back                 - Get back to Main Menu\n";
    cout << "  exit                 - Exit the shell\n";
}

void pMenu() {
    cout << "\n--- Process Management Menu ---\n";

    cout << "\nAvailable Commands:\n";
    cout << "  fork <process>            - Create a new child process using fork()\n";
    cout << "  pid              - Display the current process ID (PID)\n";
    cout << "  ppid             - Display the parent process ID (PPID)\n";
    cout << "  wait             - Wait for a child process to terminate\n";
    cout << "  ps               - Display currently running processes (like 'ps')\n";

    cout << endl;
    cout << "\n\n";
    cout << "  help             - Show this help menu\n";
    cout << "  clear            - Clear the screen\n";
    cout << "  back             - Get back to Main Menu\n";
    cout << "  exit             - Exit the shell\n";
}

void processMenu() {
    system("clear");
    pMenu();
    
    string line;
    while (true) {
        cout << "process> ";
        getline(cin, line);
        auto tokens = split(line);
        if (tokens.empty()) continue;
        string cmd = tokens[0];

        if (cmd == "fork" && tokens.size() == 2) {
            system("clear");
            forkProcess(tokens[1]);
        } else if (cmd == "pid" && tokens.size() == 1) {
            system("clear");
            showPid();
        } else if (cmd == "ppid" && tokens.size() == 1) {
            system("clear");
            showPpid();
        } else if (cmd == "wait" && tokens.size() == 1) {
            system("clear");
            waitForChild();
        } else if (cmd == "ps" && tokens.size() == 1) {
            system("clear");
            listProcesses();
        } else if (cmd == "clear" && tokens.size() == 1) {
            system("clear");
        } else if (cmd == "help" && tokens.size() == 1) {
            system("clear");
            pMenu();
        } else if (cmd == "back") {
            cout << "Returning to main menu...\n";
            break;
        } else {
            cout << "Invalid process command. Try again.\n";
        }
    }
}


void directoryMenu() {
    system("clear");
    dMenu();
    string line;
    while (true) {
        cout << "Directory> ";
        getline(cin, line);
        auto tokens = split(line);
        if (tokens.empty()) continue;
        string cmd = tokens[0];

        if (cmd == "make" && tokens.size() == 2) {
            system("clear");
            makeDirectory(tokens[1]);
        } 
        else if (cmd == "cd" && tokens.size() == 2) {
            system("clear");
            changeDirectory(tokens[1]);
        } else if (cmd == "remove" && tokens.size() == 2) {
            system("clear");
            removeDirectory(tokens[1]);
        } else if (cmd == "ls" && tokens.size() == 1) {
            system("clear");
            listFiles();
        } else if (cmd == "pwd" && tokens.size() == 1) {
            system("clear");
            printWorkingDirectory();
        } else if (cmd == "help" && tokens.size() == 1) {
            system("clear");
            dMenu();
        } else if (cmd == "clear" && tokens.size() == 1) {
            system("clear");
        }
        else if (cmd == "back") {
            system("clear");
            cout << "Returning to main menu...\n";
            break;
        } else if (cmd == "exit") {
            system("clear");
            cout << "Exiting ...\n\n";
            exit(0);
        }

        else {
            cout << "Invalid directory command. Try again.\n";
        }
    }
}

void ensureFilesDir() {
    struct stat st = {0};
    if (stat(CURRENT_DIR.c_str(), &st) == -1) {
        mkdir(CURRENT_DIR.c_str(), 0700);
    }
}

int main() {
    ensureFilesDir();
    system("clear");
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
            system("clear");
            helpMenu();
        } else if (cmd == "clear") {
            system("clear");
        } else if (cmd == "exit") {
            system("clear");
            cout << "Exiting shell. Goodbye!\n";
            break;
        } else {
            cout << "Unknown command. Type 'help' for list of commands.\n";
        }
    }

    return 0;
}
