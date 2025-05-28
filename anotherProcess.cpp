#include <iostream>
#include <string>
#include <unistd.h>

using namespace std;


int main() {
    cout << "This is another process that can be launched from the main program.\n";
    
    cout << "For demonstration, this process will simply print its PID and exit.\n";
    cout << "Current Process ID: " << getpid() << "\n";
    cout << "The parent Process ID: " << getppid() << "\n";
    sleep(5); 
    cout << "Exiting another process...\n";

    return 0;
}