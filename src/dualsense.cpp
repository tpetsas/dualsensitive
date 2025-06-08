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
#include <algorithm>

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

#define TRIGGER_BUFFER_SZ 11


// inner function to be callse by processTriggerSetting() in DS5_Output.cpp
void setTriggerProfile(unsigned char *buffer, TriggerProfile profile, std::vector<uint8_t> extras) {
    int lastIdx = 0;
    switch (profile) {
        case TriggerProfile::GameCube:
            buffer[0] = static_cast<unsigned char>(TriggerMode::Pulse);
            buffer[1] = 144;
            buffer[2] = 160;
            buffer[3] = 255;
            lastIdx = 3;
            break;
        case TriggerProfile::VerySoft:
            buffer[0] = static_cast<unsigned char>(TriggerMode::Pulse);
            buffer[1] = 128;
            buffer[2] = 160;
            buffer[3] = 255;
            lastIdx = 3;
            break;
        case TriggerProfile::Soft:
            buffer[0] = static_cast<unsigned char>(TriggerMode::Rigid_A);
            buffer[1] = 69;
            buffer[2] = 160;
            buffer[3] = 255;
            lastIdx = 3;
            break;
        case TriggerProfile::Medium:
            buffer[0] = static_cast<unsigned char>(TriggerMode::Pulse_A);
            buffer[1] = 2;
            buffer[2] = 35;
            buffer[3] = 1;
            buffer[4] = 6;
            buffer[5] = 6;
            buffer[6] = 1;
            buffer[7] = 33;
            lastIdx = 7;
            break;
        case TriggerProfile::Hard:
            buffer[0] = static_cast<unsigned char>(TriggerMode::Rigid_A);
            buffer[1] = 32;
            buffer[2] = 160;
            buffer[3] = 255;
            buffer[4] = 255;
            buffer[5] = 255;
            buffer[6] = 255;
            buffer[7] = 255;
            lastIdx = 7;
            break;
        case TriggerProfile::VeryHard:
            buffer[0] = static_cast<unsigned char>(TriggerMode::Rigid_A);
            buffer[1] = 16;
            buffer[2] = 160;
            buffer[3] = 255;
            buffer[4] = 255;
            buffer[5] = 255;
            buffer[6] = 255;
            buffer[7] = 255;
            lastIdx = 7;
            break;
        case TriggerProfile::Hardest:
            buffer[0] = static_cast<unsigned char>(TriggerMode::Pulse);
            buffer[1] = 0;
            buffer[2] = 255;
            buffer[3] = 255;
            buffer[4] = 255;
            buffer[5] = 255;
            buffer[6] = 255;
            buffer[7] = 255;
            lastIdx = 7;
            break;
        case TriggerProfile::Rigid:
            buffer[0] = static_cast<unsigned char>(TriggerMode::Rigid);
            buffer[1] = 0;
            buffer[2] = 255;
            buffer[3] = 0;
            lastIdx = 3;
            break;
        case TriggerProfile::Choppy:
            buffer[0] = static_cast<unsigned char>(TriggerMode::Rigid_A);
            buffer[1] = 2;
            buffer[2] = 39;
            buffer[3] = 33;
            buffer[4] = 39;
            buffer[5] = 38;
            buffer[6] = 2;
            lastIdx = 6;
            break;
        case TriggerProfile::VibrateTrigger:
        case TriggerProfile::VibrateTriggerPulse:
            buffer[0] = static_cast<unsigned char>(TriggerMode::Pulse_AB);
            buffer[1] = 37;
            buffer[2] = 35;
            buffer[3] = 6;
            buffer[4] = 39;
            buffer[5] = 33;
            buffer[6] = 35;
            buffer[7] = 34;
            lastIdx = 7;
            break;
        case TriggerProfile::VibrateTrigger10Hz:
            buffer[0] = static_cast<unsigned char>(TriggerMode::Pulse_B);
            buffer[1] = 10;
            buffer[2] = 255;
            buffer[3] = 40;
            lastIdx = 3;
            break;
        case TriggerProfile::Bow:
            {
                uint8_t start = extras[0];
                uint8_t end = extras[1];
                uint8_t force = extras[2];
                uint8_t snapForce = extras[3];
                buffer[0] = static_cast<unsigned char>(TriggerMode::Pulse_A);
                if (start <= 8 && end <= 8 && start < end && force <= 8 && snapForce <= 8 && end > 0 && force > 0 && snapForce > 0) {
                    uint16_t num = static_cast<uint16_t>((1 << start) | (1 << end));
                    uint32_t num2 = static_cast<uint32_t>(((force - 1) & 7) | (((snapForce - 1) & 7) << 3));
                    buffer[1] = static_cast<unsigned char>(num & 0xFF);
                    buffer[2] = static_cast<unsigned char>((num >> 8) & 0xFF);
                    buffer[3] = static_cast<unsigned char>(num2 & 0xFF);
                    buffer[4] = static_cast<unsigned char>((num2 >> 8) & 0xFF);
                    lastIdx = 4;
                }
            }
            break;
        case TriggerProfile::Resistance:
            {
                uint8_t start = extras[0];
                uint8_t force = extras[1];
                buffer[0] = static_cast<unsigned char>(TriggerMode::Rigid_B);
                if (start <= 9 && force <= 8 && force > 0) {
                    uint8_t b = (force - 1) & 7;
                    uint32_t num = 0; uint16_t num2 = 0;
                    for (int i = static_cast<int>(start); i < 10; ++i) {
                        num |= (b << (3 * i));
                        num2 |= (1 << i);
                    }
                    buffer[1] = static_cast<uint8_t>(num2 & 0xFF);
                    buffer[2] = static_cast<uint8_t>((num2 >> 8) & 0xFF);
                    buffer[3] = static_cast<uint8_t>(num & 0xFF);
                    buffer[4] = static_cast<uint8_t>((num >> 8) & 0xFF);
                    buffer[5] = static_cast<uint8_t>((num >> 16) & 0xFF);
                    buffer[6] = static_cast<uint8_t>((num >> 24) & 0xFF);
                    lastIdx = 6;
                }
            }
            break;
        case TriggerProfile::Galloping:
            {
                uint8_t start = extras[0];
                uint8_t end = extras[1];
                uint8_t firstFoot = extras[2];
                uint8_t secondFoot = extras[3];
                uint8_t frequency = extras[4];
                buffer[0] = static_cast<unsigned char>(TriggerMode::Pulse_A2);
                if (start <= 8 && end <= 9 && start < end && secondFoot <= 7 && firstFoot <= 6 && firstFoot < secondFoot && frequency > 0) {
                    uint16_t mask = (1 << start) | (1 << end);
                    uint32_t f = (secondFoot & 7) | ((firstFoot & 7) << 3);
                    buffer[1] = static_cast<uint8_t>(mask & 0xFF);
                    buffer[2] = static_cast<uint8_t>((mask >> 8) & 0xFF);
                    buffer[3] = static_cast<uint8_t>(f);
                    buffer[4] = frequency;
                    lastIdx = 4;
                }
            }
            break;
        case TriggerProfile::Machine:
            {
                uint8_t start = extras[0];
                uint8_t end = extras[1];
                uint8_t strengthA = extras[2];
                uint8_t strengthB = extras[3];
                uint8_t frequency = extras[4];
                uint8_t period = extras[5];
                buffer[0] = static_cast<unsigned char>(TriggerMode::Pulse_AB);
                if (start <= 8 && end <= 9 && end > start && strengthA <= 7 && strengthB <= 7 && frequency > 0) {
                    uint16_t mask = (1 << start) | (1 << end);
                    uint32_t f = (strengthA & 7) | ((strengthB & 7) << 3);
                    buffer[1] = static_cast<uint8_t>(mask & 0xFF);
                    buffer[2] = static_cast<uint8_t>((mask >> 8) & 0xFF);
                    buffer[3] = static_cast<uint8_t>(f);
                    buffer[4] = frequency;
                    buffer[5] = period;
                    lastIdx = 5;
                }
            }
            break;
        case TriggerProfile::Feedback:
            {
                buffer[0] = static_cast<unsigned char>(TriggerMode::Rigid_A);
                uint8_t position = extras[0];
                uint8_t strength = extras[1];
                if (position <= 9 && strength <= 8) {
                    if (strength > 0) {
                        uint8_t b = (strength - 1) & 7;
                        uint32_t num = 0;
                        uint16_t num2 = 0;
                        for (int i = position; i < 10; i++) {
                            num |= static_cast<uint32_t>(b) << (3 * i);
                            num2 |= static_cast<uint16_t>(1 << i);
                        }
                        buffer[1] = static_cast<uint8_t>(num2 & 0xFF);
                        buffer[2] = static_cast<uint8_t>((num2 >> 8) & 0xFF);
                        buffer[3] = static_cast<uint8_t>(num & 0xFF);
                        buffer[4] = static_cast<uint8_t>((num >> 8) & 0xFF);
                        buffer[5] = static_cast<uint8_t>((num >> 16) & 0xFF);
                        buffer[6] = static_cast<uint8_t>((num >> 24) & 0xFF);
                        lastIdx = 6;
                    }
                }
            }
            break;
        case TriggerProfile::Vibration:
            {
                buffer[0] = static_cast<unsigned char>(TriggerMode::Vibration);
                uint8_t position = extras[0];
                uint8_t amplitude = extras[1];
                uint8_t frequency = extras[2];
                if (position <= 9 && amplitude <= 10 && amplitude > 0 && frequency > 0) {
                    uint8_t b = (amplitude - 1) & 7;
                    uint32_t num = 0;
                    uint16_t num2 = 0;
                    for (int i = position; i < 10; i++) {
                        num |= static_cast<uint32_t>(b) << (3 * i);
                        num2 |= static_cast<uint16_t>(1 << i);
                    }
                    buffer[1] = static_cast<uint8_t>(num2 & 0xFF);
                    buffer[2] = static_cast<uint8_t>((num2 >> 8) & 0xFF);
                    buffer[3] = static_cast<uint8_t>(num & 0xFF);
                    buffer[4] = static_cast<uint8_t>((num >> 8) & 0xFF);
                    buffer[5] = static_cast<uint8_t>((num >> 16) & 0xFF);
                    buffer[6] = static_cast<uint8_t>((num >> 24) & 0xFF);
                    // skip 7 and  8
                    buffer[9] = frequency;
                    lastIdx = 9;
                }
            }
            break;
        case TriggerProfile::SlopeFeedback:
            {
                uint8_t startPosition = extras[0];
                uint8_t endPosition = extras[1];
                uint8_t startStrength = extras[2];
                uint8_t endStrength = extras[3];
                buffer[0] = static_cast<unsigned char>(TriggerMode::Rigid_A);
                if (startPosition <= 8 && startPosition >= 0 && endPosition <= 9 && endPosition > startPosition && startStrength <= 8 && startStrength >= 1 && endStrength <= 8 && endStrength >= 1) {
                    uint8_t array[10] = { 0 };
                    float slope = static_cast<float>(endStrength - startStrength) / static_cast<float>(endPosition - startPosition);
                    for (int i = startPosition; i < 10; i++) {
                        if (i <= endPosition) {
                            float strengthFloat = static_cast<float>(startStrength) + slope * static_cast<float>(i - startPosition);
                            array[i] = static_cast<uint8_t>(std::round(strengthFloat));
                        } else {
                            array[i] = endStrength;
                        }
                    }
                    bool anyStrength = false;
                    for (int i = 0; i < 10; i++) {
                        if (array[i] > 0) anyStrength = true;
                    }
                    if (anyStrength) {
                        uint32_t num = 0;
                        uint16_t num2 = 0;
                        for (int i = 0; i < 10; i++) {
                            if (array[i] > 0) {
                                uint8_t b = (array[i] - 1) & 7;
                                num |= static_cast<uint32_t>(b) << (3 * i);
                                num2 |= static_cast<uint16_t>(1 << i);
                            }
                        }
                        buffer[1] = static_cast<uint8_t>(num2 & 0xFF);
                        buffer[2] = static_cast<uint8_t>((num2 >> 8) & 0xFF);
                        buffer[3] = static_cast<uint8_t>(num & 0xFF);
                        buffer[4] = static_cast<uint8_t>((num >> 8) & 0xFF);
                        buffer[5] = static_cast<uint8_t>((num >> 16) & 0xFF);
                        buffer[6] = static_cast<uint8_t>((num >> 24) & 0xFF);
                        lastIdx = 6;
                    }
                }
            }
            break;
        case TriggerProfile::MultiplePositionFeeback:
            {
                uint8_t strength[10] = {0};
                uint32_t num = 0;
                uint16_t num2 = 0;
                buffer[0] = static_cast<unsigned char>(TriggerMode::Rigid_A);
                for (int i = 0; i < 10 && i + 1 < extras.size(); ++i) {
                    strength[i] = extras[i];
                }
                for (int i = 0; i < 10; i++) {
                    if (strength[i] > 0) {
                        uint8_t b = (strength[i] - 1) & 7;
                        num |= static_cast<uint32_t>(b) << (3 * i);
                        num2 |= static_cast<uint16_t>(1 << i);
                    }
                }
                buffer[1] = static_cast<unsigned char>(num2 & 0xFF);
                buffer[2] = static_cast<unsigned char>((num2 >> 8) & 0xFF);
                buffer[3] = static_cast<unsigned char>(num & 0xFF);
                buffer[4] = static_cast<unsigned char>((num >> 8) & 0xFF);
                buffer[5] = static_cast<unsigned char>((num >> 16) & 0xFF);
                buffer[6] = static_cast<unsigned char>((num >> 24) & 0xFF);
                lastIdx = 6;
            }
            break;
        case TriggerProfile::MultiplePositionVibration:
            {
                uint8_t frequency = extras[0];
                uint8_t amplitudes[10];
                std::copy(extras.begin() + 1, extras.begin() + 11, amplitudes);
                bool anyAmplitude = std::any_of(amplitudes, amplitudes + 10, [](uint8_t a) { return a > 0; });
                buffer[0] = static_cast<unsigned char>(TriggerMode::Pulse_B2);
                if (frequency > 0 && anyAmplitude) {
                    uint32_t num = 0;
                    uint16_t num2 = 0;
                    for (int i = 0; i < 10; ++i) {
                        if (amplitudes[i] > 0) {
                            uint8_t b = (amplitudes[i] - 1) & 7;
                            num |= static_cast<uint32_t>(b) << (3 * i);
                            num2 |= static_cast<uint16_t>(1 << i);
                        }
                    }
                    buffer[1] = static_cast<uint8_t>(num2 & 0xFF);
                    buffer[2] = static_cast<uint8_t>((num2 >> 8) & 0xFF);
                    buffer[3] = static_cast<uint8_t>(num & 0xFF);
                    buffer[4] = static_cast<uint8_t>((num >> 8) & 0xFF);
                    buffer[5] = static_cast<uint8_t>((num >> 16) & 0xFF);
                    buffer[6] = static_cast<uint8_t>((num >> 24) & 0xFF);
                    buffer[7] = 0;
                    buffer[8] = 0;
                    buffer[9] = frequency;
                    lastIdx = 9;
                }
            }
            break;
        case TriggerProfile::Weapon:
            {
                uint8_t startPosition = extras[0];
                uint8_t endPosition = extras[1];
                uint8_t strength = extras[2];
                buffer[0] = static_cast<unsigned char>(TriggerMode::Rigid_AB);
                if (startPosition <= 7 && startPosition >= 2 &&
                    endPosition <= 8 && endPosition > startPosition &&
                    strength <= 8) {
                    if (strength > 0) {
                        uint16_t num = static_cast<uint16_t>((1 << startPosition) | (1 << endPosition));
                        buffer[1] = static_cast<unsigned char>(num & 0xFF);
                        buffer[2] = static_cast<unsigned char>((num >> 8) & 0xFF);
                        buffer[3] = strength - 1;
                        lastIdx = 3;
                    }
                }
            }
            break;
        case TriggerProfile::SemiAutomaticGun:
            {
                uint8_t start = extras[0];
                uint8_t end = extras[1];
                uint8_t force = extras[2];
                buffer[0] = static_cast<unsigned char>(TriggerMode::Rigid_AB);
                if (start <= 7 && start >= 2 && end <= 8 && end > start && force <= 8 && force > 0) {
                    uint16_t num = static_cast<uint16_t>((1 << start) | (1 << end));
                    buffer[1] = static_cast<unsigned char>(num & 0xFF);
                    buffer[2] = static_cast<unsigned char>((num >> 8) & 0xFF);
                    buffer[3] = force - 1;
                    lastIdx = 3;
                }
            }
            break;
        case TriggerProfile::AutomaticGun:
            {
                uint8_t start = extras[0];
                uint8_t strength = extras[1];
                uint8_t frequency = extras[2];
                buffer[0] = static_cast<unsigned char>(TriggerMode::Pulse_B2);
                if (start <= 9 && strength <= 8 && strength > 0 && frequency > 0) {
                    uint8_t b = (strength - 1) & 7;
                    uint32_t num = 0;
                    uint16_t num2 = 0;
                    for (int i = static_cast<int>(start); i < 10; i++) {
                        num |= static_cast<uint32_t>(b) << (3 * i);
                        num2 |= static_cast<uint16_t>(1 << i);
                    }
                    buffer[1] = static_cast<uint8_t>(num2 & 0xFF);
                    buffer[2] = static_cast<uint8_t>((num2 >> 8) & 0xFF);
                    buffer[3] = static_cast<uint8_t>(num & 0xFF);
                    buffer[4] = static_cast<uint8_t>((num >> 8) & 0xFF);
                    buffer[5] = static_cast<uint8_t>((num >> 16) & 0xFF);
                    buffer[6] = static_cast<uint8_t>((num >> 24) & 0xFF);
                    buffer[8] = frequency;
                    lastIdx = 8;
                }
            }
            break;
        case TriggerProfile::Custom:
            {
                // First byte of extras determines TriggerMode (0–16 for predefined values)
                buffer[0] = static_cast<unsigned char>(extras[0]); // TriggerMode
                for (int i = 1; i <= 7 && i < extras.size(); ++i) {
                    buffer[i] = extras[i]; // Next 7 bytes are force parameters
                }
                lastIdx = 7;
            }
            break;
        case TriggerProfile::Normal:
        default:
            buffer[0] = static_cast<unsigned char>(TriggerMode::Rigid_B);
            lastIdx = 0;
            break;
    }
    for (int i = lastIdx + 1; i < TRIGGER_BUFFER_SZ; i++) {
        buffer[i] = 0;
    }
}

// create a vector by adding first as the first elment and and the provided
// vector as the rest
template<typename T>
std::vector<T> prepend(const T& first, const std::vector<T>& rest) {
    std::vector<T> result;
    result.reserve(1 + rest.size());
    result.push_back(first);
    result.insert(result.end(), rest.begin(), rest.end());
    return result;
}


namespace dualsense {

    // support a single controller for now
	DS5W::DeviceContext controller;
    // structure to keep the state to send out to controller
	DS5W::DS5OutputState outState;

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
		ZeroMemory(&outState, sizeof(DS5W::DS5OutputState));
        // make sure to enable the new trigger structure
        outState.triggerSettingEnabled = true;
        ERROR_PRINT("Init failed");
        return Status::InitFailed;
    }
    
    void terminate(void) {
        setLeftTrigger(TriggerProfile::Normal);
        setRightTrigger(TriggerProfile::Normal);
        sendState();
        DS5W::freeDeviceContext(&controller);
    }
    
    void setLeftTrigger(TriggerProfile triggerProfile, std::vector<uint8_t> extras) {
        outState.triggerSettingEnabled = true;
        outState.leftTriggerSetting.profile = triggerProfile;
        outState.leftTriggerSetting.extras = extras;
    }

    void setRightTrigger(TriggerProfile triggerProfile, std::vector<uint8_t> extras) {
        outState.triggerSettingEnabled = true;
        outState.rightTriggerSetting.profile = triggerProfile;
        outState.rightTriggerSetting.extras = extras;
    }

    void setLeftCustomTrigger(TriggerMode customMode,
                                        std::vector<uint8_t> extras) {
        outState.triggerSettingEnabled = true;
        outState.leftTriggerSetting.profile = TriggerProfile::Custom;
        std::vector<uint8_t> combined = prepend(static_cast<uint8_t>(customMode), extras);
        outState.leftTriggerSetting.extras = combined;
    }
    void setRightCustomTrigger(TriggerMode customMode,
                                        std::vector<uint8_t> extras) {
        outState.triggerSettingEnabled = true;
        outState.rightTriggerSetting.profile = TriggerProfile::Custom;
        std::vector<uint8_t> combined = prepend(static_cast<uint8_t>(customMode), extras);
        outState.rightTriggerSetting.extras = combined;
    }
    void sendState() {
	    DS5W::setDeviceOutputState(&controller, &outState);
    }
}
