/*
    dualsense.h is part of dualsense-cpp
    https://github.com/tpetsas/dualsense-cpp

    Contributors of this file:
    05.2025 Thanasis Petsas

    Licensed under the MIT License
*/

#include <log.h>
#include <dualsense.h>

#include <Windows.h>

#include <string>
#include <sstream>
#include <IO.h>
#include <Device.h>
#include <Helpers.h>
#include <iostream>

#define DEVICE_ENUM_INFO_SZ 16
#define CONTROLLER_LIMIT 16


// utils

std::string wstring_to_utf8(const std::wstring& ws) {
    int len = WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1,
                                   nullptr, 0, nullptr, nullptr);
    std::string s(len, 0);
    WideCharToMultiByte (
            CP_UTF8, 0, ws.c_str(), -1, &s[0], len, nullptr, nullptr
    );
    s.resize(len - 1);
    return s;
}

int scanControllers(std::vector<DS5W::DeviceEnumInfo>& infosVector) {
	// list all DualSense controllers
	DS5W::DeviceEnumInfo infos[CONTROLLER_LIMIT];
	unsigned int controllersCount = 0;
	DS5W_ReturnValue rv = DS5W::enumDevices (
        infos, DEVICE_ENUM_INFO_SZ, &controllersCount
    );

	if (controllersCount == 0) {
		ERROR_PRINT("No DualSense controllers found!");
		system("pause");
		return -1;
	}

	// print all controllers
	DEBUG_PRINT("Found " << controllersCount << " DualSense Controller(s):");


	// Iterate controllers
	for (unsigned int i = 0; i < controllersCount; i++) {
        infosVector.push_back(infos[i]);
        DEBUG_PRINT(
            ((infos[i]._internal.connection == DS5W::DeviceConnection::BT) ?
            std::string("Wireless (Bluetooth)") :
			std::string("Wired (USB)"))
            << " controller ("
            << wstring_to_utf8(std::wstring(infos[i]._internal.path))
            << ")"
        );
	}
    return 0;
}

#define TRIGGER_EXTRAS_SZ 11

void setTrigger(TriggerProfile profile) {
    unsigned char buffer[TRIGGER_EXTRAS_SZ] = {};
    int i=0;

    switch (profile) {
        case TriggerProfile::GameCube:
            buffer[0] = static_cast<unsigned char>(TriggerMode::Pulse);
            buffer[1] = 144;
            buffer[2] = 160;
            buffer[3] = 255;
            break;
        case TriggerProfile::VerySoft:
            buffer[0] = static_cast<unsigned char>(TriggerMode::Pulse);
            buffer[1] = 128;
            buffer[2] = 160;
            buffer[3] = 255;
            break;
        case TriggerProfile::Soft:
        case TriggerProfile::Medium:
        case TriggerProfile::Hard:
        case TriggerProfile::VeryHard:
        case TriggerProfile::Hardest:
        case TriggerProfile::Rigid:
        case TriggerProfile::Choppy:
        case TriggerProfile::VibrateTrigger:
        case TriggerProfile::VibrateTriggerPulse:
        case TriggerProfile::Resistance:
        case TriggerProfile::Galloping:
        case TriggerProfile::Machine:
        case TriggerProfile::Feedback:
        case TriggerProfile::Vibration:
        case TriggerProfile::VibrateTrigger10Hz:
        case TriggerProfile::SlopeFeedback:
        case TriggerProfile::MultiplePositionFeeback:
        case TriggerProfile::MultiplePositionVibration:
        case TriggerProfile::Bow:
        case TriggerProfile::weapon:
        case TriggerProfile::SemiAutomaticGun:
        case TriggerProfile::AutomaticGun:
        case TriggerProfile::Custom:
        case TriggerProfile::Neutral:
            [[fallthrough]];
        default:
            break;
    }
}

namespace dualsense {

    // support a single controller for now
	DS5W::DeviceContext controller;

    bool isConnected(void) {
	    DS5W::DS5InputState inState;
        return DS5W_SUCCESS(DS5W::getDeviceInputState(&controller, &inState));
    }

    void ensureConnected(void) {
        if (!isConnected()) {
            ERROR_PRINT("Device disconnected! Try to reconnect");
            DS5W::reconnectDevice(&controller);
        }
    }

    Status init(void) {
        std::vector<DS5W::DeviceEnumInfo> controllersInfo;
        if (scanControllers(controllersInfo) != 0) {
            return Status::NoControllersDetected;
        }
        // set the first dualsense found as our main controller
	    if (DS5W_SUCCESS (
            DS5W::initDeviceContext(&controllersInfo[0], &controller) )) {
            DEBUG_PRINT("DualSense controller connnected");
            return Status::Ok;
        }
        ERROR_PRINT("Init failed");
        return Status::InitFailed;
    }
    
    void terminate(void) {
        DS5W::freeDeviceContext(&controller);
    }
    
    void setLeftTrigger(TriggerMode triggerMode, std::vector<int> extras) {
        return;
    }

    void setRightTrigger(TriggerMode triggerMode, std::vector<int> extras) {
        return;
    }
}
