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

[DualSenseX-github]: https://github.com/Paliverse/DualSenseX
[DualSenseY-github]: https://github.com/WujekFoliarz/DualSenseY-v2

The following list contains all the new features that the present project introduces on top of [DualSense-Windows][Dualsense-github]:

- **Additional Trigger Profiles**
  Based on personal experimentation.

- **Custom Trigger Mode**
  Inspired by [DualSenseX][DualSenseX-github] and [DualSenseY][DualSenseY-github], allowing fully customizable trigger behavior.

- **Logging Support**
  Introduced structured logging for better debugging and diagnostics.

- **Agent Modes**
  Introduced three operational modes:
  - **SOLO**: Direct access to the DualSense controller (same as before)
  - **SERVER**: Background service managing the controller
  - **CLIENT**: Forwards trigger settings to the server via UDP

- **Tray Application (SERVER Mode)**
  The server now runs as a system tray app, including an option to enable or disable adaptive triggers on demand.

- **Automatic Service Shutdown (CLIENT/SERVER Modes)**
  The server can automatically terminate itself when the client exits, using a monitoring thread that checks the client’s PID.

- **Improved Reconnect Logic**
  Added mutex-based synchronization to enhance stability during device reconnection.

## Build Instructions

```bash
mkdir build; cd build; cmake .. -G "Visual Studio 17 2022"; cmake --build . --config Release;
```

## Tray Application Options

The Tray Application when DualSensitive is running on SERVER mode will be in the system tray with the DualSensitive icon as shown here:

There are only two options available at the moment:
- **Enable/Disable Adaptive Triggers**
Ability to enable or disable the Adaptive Triggers settings via the following provided by right clicking the DualSensitive tray icon.

Here's an example of how the icon will be shown when the "Disable Adaptive Triggers" is selected:

When the "Enable Adaptive Triggers" option is selected again, the tray icon should go back to normal:

- **Exit**
The other option is "Exit" which will terminate the service.
## API Example

This is an example of using the DualSensitive API in a standalone terminal app (i.e., SOLO mode):

```
#include <Windows.h>

#include <string>
#include <sstream>

#include <IO.h>
#include <Device.h>
#include <Helpers.h>
#include <iostream>
#include <dualsensitive.h>

int main(int argc, char** argv){
    dualsensitive::init();
    dualsensitive::ensureConnected();
    std::cout << "is controller connected: " << dualsensitive::isConnected() << std::endl;

    // Weapon profiles
    // XXX NOTE: Those are created by me empirically, so they are not accurate

    std::cout << "mode changed to soft" << std::endl;
    dualsensitive::setLeftTrigger(TriggerProfile::Choppy);
    dualsensitive::setRightTrigger(TriggerProfile::Soft);
    dualsensitive::sendState();
    Sleep(4000);

    std::cout << "mode changed to shotgun" << std::endl;
    dualsensitive::setLeftCustomTrigger(TriggerMode::Rigid_A, {60, 71, 56, 128, 195, 210, 255});
    dualsensitive::setRightTrigger(TriggerProfile::SlopeFeedback, {0, 5, 1, 8});
    dualsensitive::sendState();
    Sleep(4000);

    std::cout << "mode changed to SMG" << std::endl;
    dualsensitive::setLeftCustomTrigger(TriggerMode::Rigid_A, {71, 96, 128, 128, 128, 128, 128} );
    dualsensitive::setRightTrigger(TriggerProfile::Vibration, {3, 4, 14} );
    dualsensitive::sendState();
    Sleep(4000);

    std::cout << "mode changed to railgun" << std::endl;
    dualsensitive::setLeftTrigger(TriggerProfile::Machine, {1, 8, 3, 3, 184, 0} );
    dualsensitive::setRightCustomTrigger(TriggerMode::Pulse_B, {238, 215, 66, 120, 43, 160, 215});
    dualsensitive::sendState();
    Sleep(4000);

    std::cout << "mode changed to rocket launcher (triple-shot)" << std::endl;
    dualsensitive::setLeftCustomTrigger(TriggerMode::Rigid, {} );
    dualsensitive::setRightCustomTrigger(TriggerMode::Rigid_A, {209, 42, 232, 192, 232, 209, 232} );
    dualsensitive::sendState();
    Sleep(4000);

    std::cout << "mode changed to sticky launcher" << std::endl;
    dualsensitive::setLeftTrigger(TriggerProfile::Feedback, {3, 3} );
    dualsensitive::setRightTrigger(TriggerProfile::VeryHard, {} );
    dualsensitive::sendState();
    Sleep(4000);

    std::cout << "mode changed to pistol (with silencer)" << std::endl;
    dualsensitive::setLeftTrigger(TriggerProfile::Choppy);
    dualsensitive::setRightTrigger(TriggerProfile::Weapon, {2, 5, 5});
    dualsensitive::sendState();
    Sleep(4000);

    std::cout << "mode changed to pistol" << std::endl;
    dualsensitive::setLeftTrigger(TriggerProfile::Choppy);
    dualsensitive::setRightTrigger(TriggerProfile::MultiplePositionFeeback, {4, 7, 0, 2, 4, 6, 0, 3, 6, 0});
    dualsensitive::sendState();
    Sleep(4000);

    std::cout << "mode changed to magnum" << std::endl;
    dualsensitive::setLeftTrigger(TriggerProfile::Choppy);
    dualsensitive::setRightTrigger(TriggerProfile::SlopeFeedback, {0, 8, 8, 1});
    dualsensitive::sendState();
    Sleep(4000);

    std::cout << "mode changed to Medium Machine Gun" << std::endl;
    dualsensitive::setLeftTrigger(TriggerProfile::Vibration, {1, 10, 8});
    dualsensitive::setRightTrigger(TriggerProfile::MultiplePositionVibration, {10, 0, 1, 2, 3, 4, 5, 6, 7, 8, 8});
    dualsensitive::sendState();
    Sleep(4000);

    dualsensitive::terminate();
    std::cout << "is controller connected: " << dualsensitive::isConnected() << std::endl;

    return 0;
}
```

[dualsensitive-solo-test-main]: https://github.com/tpetsas/dualsensitive/blob/main/test/solo/main.cpp

The same example can be also found in `test` directory ([test/solo/main.cpp][dualsensitive-solo-test-main]).

You can build it by:

```
rm -r build; mkdir build; cd build; cmake .. -G "Visual Studio 17 2022"; cmake --build . --config Debug; cd ..;
```

Then, run it by:

```
.\build\Debug\solo-test.exe
```

Sample output:



In the same location (i.e., `.\build\Debug\`) you can also find the library file (`dualsensitive.lib`), and the rest of the binaries: client (`client.exe`), server (`dualsensitive-service.exe`).
