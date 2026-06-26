#include <cstddef>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

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


std::string get_mem_info(bool is_swap) {
  std::ifstream file("/proc/meminfo");
  if (!file.is_open()) {
    return is_swap ? "Unknown swap" : "Unknown memory";
  }
  std::string line;
  double total_kb = 0;
  double free_or_avail_kb = 0;

  std::string total_label = is_swap ? "SwapTotal:" : "MemTotal:";
  std::string free_label = is_swap ? "SwapFree:" : "MemAvailable:";
  size_t total_len = total_label.length();
  size_t free_len = free_label.length();

  while (std::getline(file, line)) {
    if (line.find(total_label) == 0) {
      std::stringstream kb(line.substr(total_len));
      kb >> total_kb;
    }
    else if (line.find(free_label) == 0) {
      std::stringstream kb(line.substr(free_len));
      kb >> free_or_avail_kb;
    }

    if (total_kb > 0 && free_or_avail_kb > 0) {
      break;
    }
  }
  file.close();
  if (total_kb == 0) return is_swap ? "Swap Error" : "Memory Error";
  double used_kb = total_kb - free_or_avail_kb;

  double total_gb = total_kb / (1024.0 * 1024.0);
  double used_gb = used_kb / (1024.0 * 1024.0);

  std::stringstream result;
  result << std::fixed << std::setprecision(2) << used_gb << " GiB / " << total_gb << " GiB";
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

std::string get_uptime() {
  std::ifstream file("/proc/uptime");
  if (!file.is_open()) {
    return "Unknown uptime";
  }
  double uptime_seconds = 0;
  file >> uptime_seconds;
  file.close();
  if (uptime_seconds <= 0) return "Uptime Error";
  int total_seconds = static_cast<int>(uptime_seconds);
  int days = total_seconds / 86400;
  int hours = (total_seconds % 86400) / 3600;
  int minutes = (total_seconds % 3600) / 60;

  std::stringstream result;
  if (days > 0) {
    result << days << " d " << hours << " h " << minutes << " m";
  } else if (hours > 0) {
    result << hours << " h " << minutes << " m"; 
  } else {
    result << minutes << " m";
  }
  return result.str();
}

std::string get_shell() {
  const char* shell_env = std::getenv("SHELL");
  if (shell_env == nullptr) {
    return "Unknown Shell";
  }
  std::string full_path(shell_env);
  size_t last_slash = full_path.find_last_of("/");
  if (last_slash != std::string::npos) {
    return full_path.substr(last_slash + 1);
  }
  return full_path;
}

int main() {

  std::vector<std::string> art = {
    R"(         _           )",
    R"(       `/s\`         )",
    R"(      `/s+s\`        )",
    R"(     :/\++++\:       )",       
    R"(    :/osfffso\:      )",
    R"(   :/ss/   \ss\:     )",
    R"(  `/oss/\_/\so\\`    )",
    R"( `/___/     \___\`   )",
  };

  std::vector<std::string> sys_info = {
    cyan + "Kernel: " + reset + get_kernel(),
    cyan + "Uptime: " + reset + get_uptime(),
    cyan + "Shell: " + reset + get_shell(),
    cyan + "CPU: " + reset + get_cpu(),
    cyan + "Memory: " + reset + get_mem_info(false),
    cyan + "Swap: " + reset + get_mem_info(true),
  };

  size_t  max_art_width = 0;
  for (const auto& line : art) {
    max_art_width = std::max(max_art_width, line.length());
  }
  size_t padding = 3;
  size_t max_lines = std::max(art.size(), sys_info.size());
  for (size_t i = 0; i < max_lines; i++) {
    std::string art_line = "";
    if (i < art.size()) {
      art_line = art[i];
    }
    std::cout << cyan << art_line;
    size_t current_width = (i < art.size()) ? art[i].length() : 0;
    std::cout << std::string(max_art_width - current_width + padding, ' ');

    if (i < sys_info.size()) {
      std::cout << sys_info[i];
    }
    std::cout << reset << "\n";
  }
  return 0;
}
