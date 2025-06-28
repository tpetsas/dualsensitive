<img src="https://github.com/user-attachments/assets/459c5e0d-b4e8-4661-b689-9842a0bcfa3a" alt="DualSensitive-logo" width="400"/>

# DualSensitive
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


## New Features

The following list contains all the new features that the present project introduces on top of [DualSense-Windows][Dualsense-github]:
**TODO:** add list here

## Build Instructions

```bash
mkdir build; cd build; cmake .. -G "Visual Studio 17 2022"; cmake --build . --config Release;
```
