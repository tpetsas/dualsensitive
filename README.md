# dualsense-cpp
Windows C++ API for controlling the PS5 DualSense controller


## About

This project is a copy of [DualSense-Windows], by Ludwig Füchsl ([Ohjurot]).
I just added a minimal CMake build to speed-up the building process and several
adaptive trigger profiles (e.g., specific profiles for several weapon types)
that could be handy for anyone that wants to either extend their games adding
those or serve as a base for mods, game integrations, or tooling.

[DualSense-Windows](https://github.com/Ohjurot/DualSense-Windows)
[Ohjurot](https://github.com/Ohjurot)


## Build Instructions

```bash
mkdir build; cd build; cmake .. -G "Visual Studio 17 2022"; cmake --build . --config Release;
```
