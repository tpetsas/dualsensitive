<img src="https://github.com/user-attachments/assets/989911d5-23e6-403f-b346-4b589d48f186" alt="dualsense-cpp-logo" width="400"/>

# Dualsense++
Windows C++ API for controlling the PS5 DualSense controller

Built with the tools and technologies:

[![CMake](https://img.shields.io/badge/-CMake-darkslateblue?logo=cmake)](https://cmake.org/)
![C](https://img.shields.io/badge/C-A8B9CC?logo=C&logoColor=white)
![C++](https://img.shields.io/badge/-C++-darkblue?logo=cplusplus)

## About

This project is a copy of [DualSense-Windows][Dualsense-github],
by Ludwig Füchsl ([Ohjurot][Ohjurot-github]).
I just added a minimal CMake build to speed-up the building process and several
adaptive trigger profiles (e.g., specific profiles for several weapon types)
that could be handy for anyone that wants to either extend their games adding
those or serve as a base for mods, game integrations, or tooling.

[DualSense-github]: https://github.com/Ohjurot/DualSense-Windows
[Ohjurot-github]: https://github.com/Ohjurot


## Build Instructions

```bash
mkdir build; cd build; cmake .. -G "Visual Studio 17 2022"; cmake --build . --config Release;
```
