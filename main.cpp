#include <cstdio>
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
  double total_swap_kb = 0;
  double swap_free_kb = 0;

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
  result << std::fixed << std::setprecision(2) << used_gb << " GiB / " << total_gb << " GiB";
  return result.str();
}

std::string get_swap() {
   std::ifstream file("/proc/meminfo");
  if (!file.is_open()) {
    return "Unknown swap";
  }
  std::string line;
  double total_swap_kb = 0;
  double swap_free_kb = 0;

  while (std::getline(file, line)) {
    if(line.find("SwapTotal:") == 0) {
      std::stringstream kb(line.substr(10));
      kb >> total_swap_kb;
    }
    else if(line.find("SwapFree:") == 0) {
      std::stringstream kb(line.substr(9));
      kb >> swap_free_kb;
    }

    if (total_swap_kb > 0 && swap_free_kb > 0) {
      break;
    }
  }
  file.close();
  if (total_swap_kb == 0) return "Swap Error";
  double used_swap_kb = total_swap_kb - swap_free_kb;

  double total_swap_gb = total_swap_kb / (1024.0 * 1024.0);
  double swap_used_gb = used_swap_kb / (1024.0 * 1024.0);

  std::stringstream result;
  result << std::fixed << std::setprecision(2) << swap_used_gb << " GiB / " << total_swap_gb << " GiB";
  return result.str();
}

std::string get_kernel() {
  std::ifstream file("/proc/version");
  if (!file.is_open()) {
    return "Unknown kernel";
  }
  std::string line;
  std::getline(file, line);
  size_t pos = line.find("version ");
  if (pos != std::string::npos) {
    std::string end = line.substr(pos + 8);
    std::stringstream lx(end);
    std::string kernel_version;
    lx >> kernel_version;
    file.close();
    return kernel_version;
  }
  file.close();
  return "Kernel not found";
}

int main() {
  std::cout << cyan << "Kernel: " << reset << get_kernel() << std::endl;
  std::cout << cyan << "CPU: " << reset << get_cpu() << std::endl;
  std::cout << cyan << "Memory: " << reset << get_mem() << std::endl;
  std::cout << cyan << "Swap: " << reset << get_swap() << std::endl;
  return 0;
}
