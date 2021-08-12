#include "process.h"

#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"
#include "unistd.h"

using std::string;
using std::to_string;
using std::vector;

// Constructor definition
Process::Process(int pid) : pid_(pid) { CpuUtilization(); };

// Return this process's ID
int Process::Pid() { return pid_; }

// Return this process's CPU utilization
float Process::Utilization(int pid) {
  string line;
  string value;
  int i = 0;
  long int utime, stime, cutime, cstime, starttime, totaltime, seconds;
  float cpu_usage;

  std::ifstream filestream("/proc/" + std::to_string(pid) + "/stat");
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> value) {
        i += 1;
        if (i == 14) {
          utime = std::stoi(value);
        }
        if (i == 15) {
          stime = std::stoi(value);
        }
        if (i == 16) {
          cutime = std::stoi(value);
        }
        if (i == 17) {
          cstime = std::stoi(value);
        }
        if (i == 22) {
          starttime = std::stoi(value);

          // Calculation of CPU utilization
          totaltime = (utime + stime + cutime + cstime) / sysconf(_SC_CLK_TCK);
          seconds = LinuxParser::UpTime() - (starttime / sysconf(_SC_CLK_TCK));
          cpu_usage = (float)totaltime / (float)seconds;
          return cpu_usage;
        }
      }
    }
  }
  return (float)0;
}

float Process::CpuUtilization() {
  this->cpu_ = Utilization(pid_);
  return this->cpu_;
}

// Return the command that generated this process
string Process::Command() { return LinuxParser::Command(pid_); }

// Return this process's memory utilization
string Process::Ram() { return LinuxParser::Ram(pid_); }

// Return the user (name) that generated this process
string Process::User() { return LinuxParser::User(LinuxParser::Uid(pid_)); }

// Return the age of this process (in seconds)
long int Process::UpTime() { return LinuxParser::UpTime(pid_); }

// Overload the "less than" comparison operator for Process objects
bool Process::operator<(Process const& a) const { return cpu_ > a.cpu_; }