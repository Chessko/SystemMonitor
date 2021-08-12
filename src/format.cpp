#include "format.h"

#include <string>

using std::string;

// Complete this helper function
// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS

string Format::ElapsedTime(long seconds) {
  long int hours = seconds / 3600;
  long int minutes = (seconds % 3600) / (float)60;
  long int seconds_remaining = seconds % 60;
  return string(std::to_string(hours) + ":" + std::to_string(minutes) + ":" +
                std::to_string(seconds_remaining)) +  " ";
}