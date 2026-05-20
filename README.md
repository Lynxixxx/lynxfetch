# 💻 lynxfetch

My first C++ CLI tool for fetching system info. It reads data straight from the Linux kernel via `/proc` with zero dependencies.

## Features
* **Kernel**: Extracts your Linux kernel release version from `/proc/version`
* **CPU**: Parses your exact CPU model right out of `/proc/cpuinfo`
* **RAM**: Calculates used vs available memory from `/proc/meminfo` and converts it to GiB
* **Swap**: Monitors your swap partition status (`SwapTotal` and `SwapFree`)
* **Design**: Clean ANSI color formatting for your terminal output
* Nerd Fonts support/icons coming soon!

## Build & Run

To compile this, you'll just need g++:
```bash
g++ main.cpp -o lynxfetch
./lynxfetch
```
