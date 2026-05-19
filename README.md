# 💻 lynxfetch

My first C++ CLI tool for fetching system info. It reads data straight from the Linux kernel via /proc with zero dependencies.

## Features
* Parses your CPU model right out of /proc/cpuinfo
* Calculates used vs available RAM from /proc/meminfo and converts it to GiB
* Clean ANSI colors for your terminal
* Nerd Fonts support/icons coming soon

## Build & Run

To compile this, you'll just need g++:
```bash
g++ main.cpp -o lynxfetch
./lynxfetch
```
