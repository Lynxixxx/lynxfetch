#include <iomanip>
#include <ios>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

const std::string reset = "\033[0m";
const std::string cyan = "\033[36m";
const std::string magenta = "\033[35m";
const std::string blue = "\033[34m";
const std::string bold = "\033[1m";

std::string get_cpu() {
  std::ifstream file("/proc/cpuinfo");
  if (!file.is_open()) {
    return "Unknown CPU";
  }
  std::string line;
  while (std::getline(file, line)) {
    if (line.find("model name") == 0) {
      size_t colon_pos = line.find(":");
      if (colon_pos != std::string::npos) {
        std::string cpu_name = line.substr(colon_pos + 1);
        while (!cpu_name.empty() && cpu_name[0] == ' ') {
          cpu_name.erase(0, 1);
        }
        file.close();
        return cpu_name;
      }
    }
  }
  file.close();
  return "CPU not found";
}


std::string get_mem() {
  std::ifstream file("/proc/meminfo");
  if (!file.is_open()) {
    return "Unknown memory";
  }
  std::string line;
  double total_kb = 0;
  double available_kb = 0;

  while (std::getline(file, line)) {
    if (line.find("MemTotal:") == 0) {
      std::stringstream kb(line.substr(9));
      kb >> total_kb;
    }
    else if (line.find("MemAvailable:") == 0) {
      std::stringstream kb(line.substr(13));
      kb >> available_kb;
    }
    if (total_kb > 0 && available_kb > 0) {
      break;
    }
  }
  file.close();
  if (total_kb == 0) return "Memory Error";
  double used_kb = total_kb - available_kb;

  double total_gb = total_kb / (1024.0 * 1024.0);
  double used_gb = used_kb / (1024.0 * 1024.0);

  std::stringstream result;
  result << std::fixed << std::setprecision(2) << used_gb << " GiB / " << total_gb << " Gib" << std::endl;
  return result.str();
}



int main() {
  std::cout << cyan << "CPU: " << reset << get_cpu() << std::endl;
  std::cout << cyan << "Memory: " << reset << get_mem() << std::endl;
  return 0;
}
