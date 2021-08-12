#include "linux_parser.h"

#include <cmath>
#include <sstream>
#include <string>
#include <vector>

#include "dirent.h"
#include "unistd.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// Read os information from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// Read kernel information from the filesystem
string LinuxParser::Kernel() {
  string os, kernel, version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
  string line, key, value;
  int memTotal, memFree;
  float memoryUtilization;

  std::ifstream filestream(kProcDirectory + kMeminfoFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "MemTotal") {
          memTotal = std::stoi(value);
        } else if (key == "MemFree") {
          memFree = std::stoi(value);
        }
      }
      if (key == "MemFree") {
        break;
      }
    }
  }
  memoryUtilization = ((float)memTotal - (float)memFree) / (float)memTotal;
  return memoryUtilization;
}

// Read and return the system uptime
long int LinuxParser::UpTime() {
  string upTime;
  string line;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> upTime;
  }
  return std::stol(upTime);
}

// Read and return the number of jiffies for the system
long LinuxParser::Jiffies(std::string keyword) {
  string line, key;
  string user, nice, system, irq, softirq, steal;
  string idle, iowait;

  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      if (linestream >> key >> user >> nice >> system >> idle >> iowait >>
          irq >> softirq >> steal) {
        if (key == "cpu") {
          if (keyword == "total") {
            return std::stol(user) + std::stol(nice) + std::stol(system) +
                   std::stol(idle) + std::stol(iowait) + std::stol(irq) +
                   std::stol(softirq) + std::stol(steal);
          } else if (keyword == "active") {
            return std::stol(user) + std::stol(nice) + std::stol(irq) +
                   std::stol(softirq) + std::stol(steal);
          } else if (keyword == "idle") {
            return std::stol(idle) + std::stol(iowait);
          }
        }
      }
    }
  }
  return 0;
}

// Read and return the number of active jiffies for a PID
// long LinuxParser::ActiveJiffies(int pid [[maybe_unused]]) { return 0; }

// Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() { return LinuxParser::Jiffies("active"); }

// Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() { return LinuxParser::Jiffies("idle"); }

// Read and return CPU utilization
float LinuxParser::CpuUtilization() {
  float jiffies = LinuxParser::Jiffies("total");
  float idleJiffies = LinuxParser::IdleJiffies();

  return (jiffies - idleJiffies) / jiffies; // return CPU Utilization
}

// Helper function to simplify parsing some files
int LinuxParser::Processes(std::string keyword) {
  string line, key, value;

  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      if (linestream >> key >> value) {
        if (key == keyword) {
          return std::stoi(value);
        }
      }
    }
  }
  return 0;
}

// Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  return LinuxParser::Processes(std::string("processes"));
}

// Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  return LinuxParser::Processes(std::string("procs_running"));
}

// Read and return the command associated with a process
string LinuxParser::Command(int pid) {
  string line;
  string key, value;

  std::ifstream filestream("/proc/" + std::to_string(pid) + "/cmdline");
  if (filestream.is_open()) {
    std::getline(filestream, line);
    return line;
  }
  return std::string{"unknown"};
}

// Read and return the memory used by a process
string LinuxParser::Ram(int pid) {
  string line;
  string key, value;

  std::ifstream filestream("/proc/" + std::to_string(pid) + "/status");
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      if (linestream >> key >> value) {
        if (key == "VmSize") {
          return std::to_string(std::stoi(value) /
                                1024);  // convert from kB to MB
        }
      }
    }
  }
  return std::string{"unknown"};
}

// Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) {
  string line;
  string key, value;

  std::ifstream filestream("/proc/" + std::to_string(pid) + "/status");
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      if (linestream >> key >> value) {
        if (key == "Uid") {
          return value;
        }
      }
    }
  }
  return std::string{"unknown"};
}

// Read and return the user associated with a process
string LinuxParser::User(std::string uid) {
  string line;
  string username, value1, value2, value3;

  std::ifstream filestream("/etc/passwd");
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');  // original format: "username:x:userID:userID: ..."
      std::istringstream linestream(line);
      if (linestream >> username >> value1 >> value2 >> value3) {
        if (value1 == "x" && value2 == uid && value3 == uid) {
          return username;
        }
      }
    }
  }
  return std::string{"unknown"};
}

// TODO: Read and return the uptime of a process
long LinuxParser::UpTime(int pid) {
  string line;
  string value;
  int i = 0;
  long int starttime;

  std::ifstream filestream("/proc/" + std::to_string(pid) + "/stat");
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> value) {
        i += 1;
        if (i == 22) {
          starttime = int(std::stof(value) / float(sysconf(_SC_CLK_TCK)));
          return LinuxParser::UpTime() - starttime;
        }
      }
    }
  }
  return 0;
}
