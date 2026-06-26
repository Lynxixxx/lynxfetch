// BY LYNXIX
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <sys/statvfs.h>
#include <ifaddrs.h>
#include <arpa/inet.h>

const std::string reset = "\033[0m";
const std::string cyan = "\033[36m";
const std::string magenta = "\033[35m";
const std::string blue = "\033[34m";
const std::string bold = "\033[1m";

std::string run_cmd(const std::string& cmd) {
  char buffer[128];
  std::string result = "";
  FILE* pipe = popen(cmd.c_str(), "r");
  if (!pipe) return "";
  while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
    result += buffer;
  }
  pclose(pipe);
  if (!result.empty() && result.back() == '\n') {
    result.pop_back();
  }
  return result;
}

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
  std::string shell_name = (last_slash != std::string::npos) ? full_path.substr(last_slash + 1) : full_path;
  
  std::string version = run_cmd(shell_name + " --version 2>&1");
  if (!version.empty()) {
    std::stringstream ss(version);
    std::string first_token, second_token;
    ss >> first_token >> second_token;
    if (shell_name == "zsh" && !second_token.empty()) {
      shell_name += " " + second_token;
    } else if (shell_name == "bash" && !second_token.empty()) {
      size_t version_pos = version.find("version ");
      if (version_pos != std::string::npos) {
        std::stringstream bs(version.substr(version_pos + 8));
        std::string bash_ver;
        bs >> bash_ver;
        shell_name += " " + bash_ver;
      }
    } else if (!second_token.empty()) {
      shell_name += " " + second_token;
    }
  }
  return shell_name;
}

std::string get_de() {
  const char* de_env = std::getenv("XDG_CURRENT_DESKTOP");
  if (!de_env) de_env = std::getenv("DESKTOP_SESSION");
  if (!de_env) return "None";
  std::string de(de_env);
  if (de == "GNOME") {
    std::string ver = run_cmd("gnome-shell --version 2>/dev/null");
    if (!ver.empty()) return ver;
  } else if (de == "KDE") {
    std::string ver = run_cmd("plasmashell --version 2>/dev/null");
    if (!ver.empty()) return ver;
  }
  return de;
}

std::string get_wm() {
  const char* xdg_wm = std::getenv("XDG_DATA_DIRS");
  std::string wm = run_cmd("wmctrl -m 2>/dev/null | grep Name: | awk '{print $2}'");
  if (wm.empty()) {
    if (std::getenv("WAYLAND_DISPLAY")) {
      std::string xdg = std::getenv("XDG_CURRENT_DESKTOP") ? std::getenv("XDG_CURRENT_DESKTOP") : "";
      if (xdg.find("sway") != std::string::npos || xdg.find("Sway") != std::string::npos) return "Sway";
      if (xdg.find("Hyprland") != std::string::npos) return "Hyprland";
      return "Wayland Compositor";
    }
    return "Unknown WM";
  }
  return wm;
}

std::string get_display() {
  std::string res = run_cmd("xrandr 2>/dev/null | grep ' current ' | awk -F'current' '{print $2}' | awk -F',' '{print $1}'");
  if (res.empty()) {
    res = run_cmd("wlr-randr 2>/dev/null | grep 'current' | head -n 1 | awk '{print $1}'");
  }
  if (res.empty()) {
    return "Unknown Display";
  }
  while (!res.empty() && (res[0] == ' ' || res[0] == '\t')) res.erase(0, 1);
  return res;
}

std::string get_terminal() {
  const char* term = std::getenv("TERM_PROGRAM");
  if (term) return std::string(term);
  const char* term_env = std::getenv("TERM");
  if (term_env) return std::string(term_env);
  return "Unknown Terminal";
}

std::string get_term_font() {
  std::string font = run_cmd("gsettings get org.gnome.desktop.interface monospace-font-name 2>/dev/null");
  if (font.empty() || font == "No such schema") {
    font = run_cmd("fc-match -f '%{family}\n' monospace 2>/dev/null");
  }
  if (font.empty()) return "Unknown Font";
  if (font.front() == '\'' && font.back() == '\'') {
    font = font.substr(1, font.length() - 2);
  }
  return font;
}

std::string get_gpu() {
  std::string gpu = run_cmd("lspci 2>/dev/null | grep -E \"VGA|3D\" | head -n 1 | awk -F ': ' '{print $2}'");
  if (gpu.empty()) return "Unknown GPU";
  size_t bracket_pos = gpu.find("[");
  if (bracket_pos != std::string::npos) {
    size_t end_bracket = gpu.find("]", bracket_pos);
    if (end_bracket != std::string::npos) {
      return gpu.substr(bracket_pos + 1, end_bracket - bracket_pos - 1);
    }
  }
  return gpu;
}

std::string get_disk() {
  struct statvfs vfs;
  if (statvfs("/", &vfs) != 0) {
    return "Disk Error";
  }
  double total = (double)vfs.f_blocks * vfs.f_frsize;
  double free_space = (double)vfs.f_bavail * vfs.f_frsize;
  double used = total - free_space;

  double total_gb = total / (1024.0 * 1024.0 * 1024.0);
  double used_gb = used / (1024.0 * 1024.0 * 1024.0);

  std::stringstream result;
  result << std::fixed << std::setprecision(2) << used_gb << " GiB / " << total_gb << " GiB";
  return result.str();
}

std::string get_local_ip() {
  struct ifaddrs *interfaces = nullptr;
  struct ifaddrs *temp_addr = nullptr;
  std::string ip = "No IP";
  if (getifaddrs(&interfaces) == 0) {
    temp_addr = interfaces;
    while (temp_addr != nullptr) {
      if (temp_addr->ifa_addr && temp_addr->ifa_addr->sa_family == AF_INET) {
        std::string name = temp_addr->ifa_name;
        if (name != "lo" && name.find("docker") == std::string::npos && name.find("br-") == std::string::npos) {
          ip = inet_ntoa(((struct sockaddr_in*)temp_addr->ifa_addr)->sin_addr);
          break;
        }
      }
      temp_addr = temp_addr->ifa_next;
    }
  }
  freeifaddrs(interfaces);
  return ip;
}

int main() {

  std::vector<std::string> art = {
    R"(              -`                   )",
    R"(             .o+/                  )",
    R"(            `os+s:                 )",
    R"(           `ssqoo::                )",       
    R"(          \q:+sso/s:               )",
    R"(         `/qq-sso/+/`              )",
    R"(        `nqssop--qqnq`             )",
    R"(       `:\nqqooosqqq/:`            )",
    R"(      ns\qd+       +d/n:           )",
    R"(     qwsq+-`       `-sq+q          )",
    R"(    `\w+:w-`       -:ss+/`         )",
    R"(   q-\s+`            `+s/-q        )",
    R"(  `-/+                  +\-`       )",
    R"(  \`                      `/       )",


  };

  std::vector<std::string> sys_info = {
    cyan + "Kernel: " + reset + get_kernel(),
    cyan + "Uptime: " + reset + get_uptime(),
    cyan + "Shell: " + reset + get_shell(),
    cyan + "DE: " + reset + get_de(),
    cyan + "WM: " + reset + get_wm(),
    cyan + "Display: " + reset + get_display(),
    cyan + "Terminal: " + reset + get_terminal(),
    cyan + "Terminal Font: " + reset + get_term_font(),
    cyan + "CPU: " + reset + get_cpu(),
    cyan + "GPU: " + reset + get_gpu(),
    cyan + "Memory: " + reset + get_mem_info(false),
    cyan + "Swap: " + reset + get_mem_info(true),
    cyan + "Disk: " + reset + get_disk(),
    cyan + "Local IP: " + reset + get_local_ip(),
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
