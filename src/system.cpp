
#include "system.h"

#include <algorithm>
#include <cstddef>
#include <set>
#include <string>
#include <vector>

#include "linux_parser.h"
#include "process.h"
#include "processor.h"
#include "unistd.h"

using std::set;
using std::size_t;
using std::string;
using std::vector;

// Return the system's CPU
Processor& System::Cpu() { return cpu_; }

// Return a container composed of the system's processes
vector<Process>& System::Processes() {
  // Parse existing PIDs into a vector
  vector<int> pids{LinuxParser::Pids()};

  // Emplace all new processes to private system variable processes_
  for (int pid : pids) {
    if (std::find(existingPIDs.begin(), existingPIDs.end(), pid) !=
        existingPIDs.end()) {
    } else {
      Process process(pid);
      processes_.emplace_back(process);
      existingPIDs.emplace_back(pid);
    }
  }

  // Update CPU utilization of each process
  for (Process& process : processes_) {
    process.CpuUtilization();
  }

  // Sort by CPU utilization (see overloaded compare operator in
  // src/process.cpp)
  std::sort(processes_.begin(), processes_.end());

  return processes_;
}

// Return the system's kernel identifier (string)
std::string System::Kernel() { return LinuxParser::Kernel(); }

// Return the system's memory utilization
float System::MemoryUtilization() { return LinuxParser::MemoryUtilization(); }

// Return the operating system name
std::string System::OperatingSystem() { return LinuxParser::OperatingSystem(); }

// Return the number of processes actively running on the system
int System::RunningProcesses() { return LinuxParser::RunningProcesses(); }

// Return the total number of processes on the system
int System::TotalProcesses() { return LinuxParser::TotalProcesses(); }

// Return the number of seconds since the system started running
long int System::UpTime() { return LinuxParser::UpTime(); }