#include "processor.h"
#include "linux_parser.h"
#include <string>

// TODO: Return the aggregate CPU utilization
float Processor::Utilization() { 
    return LinuxParser::CpuUtilization(); 
    }