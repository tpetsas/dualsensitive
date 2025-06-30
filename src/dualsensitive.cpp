/*
    dualsensitive.h is part of DualSensitive
    https://github.com/tpetsas/dualsensitive

    Contributors of this file:
    05.2025 Thanasis Petsas

    Licensed under the MIT License
*/

#include <logger.h>
#include <dualsensitive.h>
#include <udp.h>

#include <Windows.h>

#include <string>
#include <mutex>
#include <sstream>
#include <IO.h>
#include <Device.h>
#include <Helpers.h>
#include <iostream>
#include <algorithm>

#define DEVICE_ENUM_INFO_SZ 16
#define CONTROLLER_LIMIT 16
#define TRIGGER_INDEX 0
#define PROFILE_INDEX 1
#define EXTRAS_SIZE_INDEX 2
#define MIN_PAYLOAD_SIZE 3
#define PAYLOAD_TYPE_SIZE 1
#define PID_SIZE 4
#define EXTRAS_BUFFER_INDEX 3

// for the retry logic used for connecting to the controller
#define MAX_RETRIES 5
#define RETRY_DELAY_MS 500

// utils

enum class Trigger : uint8_t {
    Left = 0,
    Right = 1
};


std::vector<uint8_t> serializeBindPayload(uint32_t pid) {
    std::vector<uint8_t> buffer;
    buffer.push_back(static_cast<uint8_t>(PayloadType::BIND));  // 1 byte
    buffer.push_back((pid >>  0) & 0xFF);
    buffer.push_back((pid >>  8) & 0xFF);
    buffer.push_back((pid >> 16) & 0xFF);
    buffer.push_back((pid >> 24) & 0xFF);
    return buffer;
}

bool deserializeBindPayload(const std::vector<uint8_t>& buffer, uint32_t& pid) {

    if (buffer.size() < PID_SIZE) {
        ERROR_PRINT("Bind PID payload too small!");
        return false;
    }
    pid = (buffer[0]) | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);
    INFO_PRINT("Bound to client PID: " << pid);
    return true;
}

std::vector<uint8_t> serializeTriggerPayload(Trigger trigger, TriggerProfile profile, const std::vector<uint8_t>& extras) {
    std::vector<uint8_t> buffer;
    buffer.push_back(static_cast<uint8_t>(PayloadType::TRIGGER));   // 1 byte
    buffer.push_back(static_cast<uint8_t>(trigger));                // 1 byte
    buffer.push_back(static_cast<int8_t>(profile));                 // 1 byte
    buffer.push_back(static_cast<uint8_t>(extras.size()));          // 1 byte
    buffer.insert(buffer.end(), extras.begin(), extras.end());      // extras
    return buffer;
}

bool deserializeTriggerPayload(const std::vector<uint8_t>& buffer, Trigger& trigger, TriggerProfile& profile, std::vector<uint8_t>& extras) {
    if (buffer.size() < MIN_PAYLOAD_SIZE) {
        ERROR_PRINT("buffer size less than expected!");
        return false;
    }
    trigger = static_cast<Trigger>(buffer[TRIGGER_INDEX]);
    profile = static_cast<TriggerProfile>(static_cast<int8_t>(buffer[PROFILE_INDEX]));
    uint8_t extrasSize = buffer[EXTRAS_SIZE_INDEX];

    if (buffer.size() < EXTRAS_BUFFER_INDEX + extrasSize) {
        ERROR_PRINT("extras found corrupted!");
        return false;
    }

    extras.assign(buffer.begin() + EXTRAS_BUFFER_INDEX, buffer.begin() + EXTRAS_BUFFER_INDEX + extrasSize);
    return true;
}

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
    DS5W::enumDevices (
        infos, DEVICE_ENUM_INFO_SZ, &controllersCount
    );

    if (controllersCount == 0) {
        ERROR_PRINT("No DualSense controllers found!");
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


namespace dualsensitive {

    // These control the active mode
    static AgentMode agentMode = AgentMode::SOLO;
    static uint16_t udpPort = 28472;
    static std::mutex initMutex;
    static bool hasInit = false;
    // only if enabled is true, the DualSense settings will be sent to
    // controller
    static std::mutex enabledMutex;
    static bool enabled = true;
    static std::mutex clientPidMutex;
    static uint32_t clientPid;

    // support a single controller for now (on SOLO and SERVER modes only)
    DS5W::DeviceContext controller;
    // structure to keep the state to send out to controller
    // (on SOLO and CLIENT modes only)
    DS5W::DS5OutputState outState;

    bool isConnected(void) {
        if (agentMode == AgentMode::CLIENT) {
            ERROR_PRINT("Not applicable in CLIENT mode");
            return false;
        }
        DS5W::DS5InputState inState;
        return DS5W_SUCCESS(DS5W::getDeviceInputState(&controller, &inState));
    }

    uint32_t getClientPid(void) {
        std::lock_guard<std::mutex> lock(clientPidMutex);
        return clientPid;
    }

    // XXX This should be called in a block where the initMutex lock has been acquired
    Status connectToController() {
        std::vector<DS5W::DeviceEnumInfo> controllersInfo;

        Status status;
        for (int attempt = 0; attempt < MAX_RETRIES; ++attempt) {
            std::this_thread::sleep_for(std::chrono::milliseconds(RETRY_DELAY_MS));
            DEBUG_PRINT("Device disconnected! Attempt " << (attempt+1) << " to reconnect...");

            if (scanControllers(controllersInfo) != 0) {
                status = Status::NoControllersDetected;
                continue;
            }
            // set the first DualSense controller found as our main controller
            if (!DS5W_SUCCESS (
                DS5W::initDeviceContext(&controllersInfo[0], &controller) )) {
                ERROR_PRINT("Init failed");
                status = Status::InitFailed;
                continue;
            }
            status = Status::Ok;
            DEBUG_PRINT("DualSense controller connnected");
            hasInit = true;
            ZeroMemory(&outState, sizeof(DS5W::DS5OutputState));

            // enable red color by default with medium intensity
            outState.lightbar = DS5W::color_R8G8B8_UCHAR_A32_FLOAT(255, 0, 0, 128);
            outState.disableLeds = false;
            break;
        }
        return status;
    }

    void ensureConnected(void) {
        if (agentMode == AgentMode::CLIENT) {
            ERROR_PRINT("Not applicable in CLIENT mode");
            return;
        }

        if (isConnected())
            return;

        std::lock_guard<std::mutex> lock(initMutex);
        if (hasInit) {
            // try to reconnect to the same controller
            ERROR_PRINT("Device disconnected! Try to reconnect");
            if (DS5W::reconnectDevice(&controller) == DS5W_OK)
                return;
        }
        connectToController();
    }

    bool assignTriggersFromPayload(const std::vector<uint8_t> payload) {
        Trigger trigger;
        TriggerProfile profile;
        std::vector<uint8_t> extras;

        if (!deserializeTriggerPayload(payload, trigger, profile, extras)) {
            ERROR_PRINT("failed to deserialize payload!");
            return false;
        }
        outState.triggerSettingEnabled = true;
        switch (trigger) {
            case Trigger::Left:
                //setLeftTrigger(profile, extras);
                outState.leftTriggerSetting.profile = profile;
                outState.leftTriggerSetting.extras = extras;
                break;
            case Trigger::Right:
                //setRightTrigger(profile, extras);
                outState.rightTriggerSetting.profile = profile;
                outState.rightTriggerSetting.extras = extras;
                break;
            default:
                ERROR_PRINT("Unknown trigger type!");
                return false;
        };
        return true;
    }

    Status init(AgentMode mode, const std::string& logPath, bool enableDebug,
            uint16_t port) {
        agentMode = mode;

        // true will enable debug output
        // TODO: add a bool parameter so we can tweak that as we want
        Logger::init(enableDebug);
        if (!Logger::setLogFile(logPath)) {
            ERROR_PRINT("Failed to set log file. Falling back to stdout.");
        } else {
            INFO_PRINT("set log file: " << logPath);
        }

        switch (mode) {
            case AgentMode::CLIENT:
                udpPort = port;
                if (udp::startClient(udpPort) != udp::Status::Success)
                    return Status::InitFailed;
                hasInit = true;
                return Status::Ok;
            case AgentMode::SERVER: {
                udpPort = port;
                auto callback = [](const std::vector<uint8_t>& payload) {
                    if (payload.empty()) {
                        ERROR_PRINT("Payload empty!");
                        return;
                    }
                    PayloadType type = static_cast<PayloadType>(payload[0]);

                    switch (type) {
                        case PayloadType::BIND: {
                            if (payload.size() < PAYLOAD_TYPE_SIZE + PID_SIZE) {
                                ERROR_PRINT("Bind PID payload too small!");
                                return;
                            }
                            // skip the payload type
                            std::vector<uint8_t> trimmed (
                                    payload.begin() + 1,
                                    payload.end()
                            );
                            {
                                std::lock_guard<std::mutex> lock(clientPidMutex);
                                if (!deserializeBindPayload(trimmed, clientPid)) {
                                    ERROR_PRINT("failed to deserialize Bind PID payload!");
                                    return;
                                }
                            }
                            break;
                        }
                        case PayloadType::TRIGGER: {
                            if (payload.size() < MIN_PAYLOAD_SIZE + PAYLOAD_TYPE_SIZE) {
                                ERROR_PRINT("Trigger payload size less than expected!");
                                return;
                            }
                            // skip the payload type
                            std::vector<uint8_t> trimmed (
                                    payload.begin() + 1,
                                    payload.end()
                            );
                            if(!assignTriggersFromPayload(trimmed)) {
                                ERROR_PRINT("Could not set triggers from payload!");
                                return;
                            }
                            sendState();
                        }
                        default:
                            ERROR_PRINT("Unknown payload type: " << static_cast<uint8_t>(type) << "!");
                    };
                };

                if (udp::startServer(udpPort, callback) != udp::Status::Success)
                    return Status::InitFailed;
                break;
            }
            case AgentMode::SOLO:
            default:
                ;
        }

        // only on SERVER and SOLO mode
        std::lock_guard<std::mutex> lock(initMutex);
        if (hasInit) {
            INFO_PRINT("DualSense controller already connnected");
            return Status::Ok;
        }
        return connectToController();
    }

    void terminate(void) {

        setLeftTrigger(TriggerProfile::Normal);
        setRightTrigger(TriggerProfile::Normal);

        switch (agentMode) {
            case AgentMode::CLIENT:
                udp::stopClient();
                return;
            case AgentMode::SERVER:
                udp::stopServer();
                break;
            case AgentMode::SOLO:
                sendState();
            default:
                ;
        }

        // only on SERVER and SOLO mode

        DS5W::freeDeviceContext(&controller);
    }

    void sendPidToServer(void) {
        if (agentMode != AgentMode::CLIENT) {
            ERROR_PRINT("sendPidToServer() is only available in CLIENT mode");
            return;
        }
        udp::send(serializeBindPayload(GetCurrentProcessId()));
    }

    void setTrigger(Trigger trigger, TriggerProfile triggerProfile,
                                                std::vector<uint8_t> extras){
        switch (agentMode) {
            case AgentMode::CLIENT: {
                std::vector<uint8_t> payload = serializeTriggerPayload(
                        trigger,
                        triggerProfile,
                        extras
                );
                udp::send(payload);
                break;
            }
            case AgentMode::SERVER:
                //ERROR_PRINT("Not applicable in SERVER mode");
                //break;
            case AgentMode::SOLO:
            default:
                outState.triggerSettingEnabled = true;
                if (trigger == Trigger::Left) {
                    outState.leftTriggerSetting.profile = triggerProfile;
                    outState.leftTriggerSetting.extras = extras;
                } else if (trigger == Trigger::Right) {
                    outState.rightTriggerSetting.profile = triggerProfile;
                    outState.rightTriggerSetting.extras = extras;
                } else {
                    ERROR_PRINT("Unknown trigger type!");
                    break;
                }
                sendState();
        }
    }

    void setLeftTrigger(TriggerProfile triggerProfile, std::vector<uint8_t> extras) {
       setTrigger(Trigger::Left, triggerProfile, extras);
    }

    void setRightTrigger(TriggerProfile triggerProfile, std::vector<uint8_t> extras) {
       setTrigger(Trigger::Right, triggerProfile, extras);
    }

    void setLeftCustomTrigger(TriggerMode customMode,
                                        std::vector<uint8_t> extras) {
        TriggerProfile profile = TriggerProfile::Custom;
        std::vector<uint8_t> extendedExtras = prepend (
                static_cast<uint8_t>(customMode),
                extras
        );
        setTrigger(Trigger::Left, profile, extendedExtras);
    }

    void setRightCustomTrigger(TriggerMode customMode,
                                        std::vector<uint8_t> extras) {
        TriggerProfile profile = TriggerProfile::Custom;
        std::vector<uint8_t> extendedExtras = prepend (
                static_cast<uint8_t>(customMode),
                extras
        );
        setTrigger(Trigger::Right, profile, extendedExtras);
    }

    void sendState(void) {
        {
            std::lock_guard<std::mutex> lock(enabledMutex);
            if (!enabled) {
                DEBUG_PRINT("DualSensitive is disabled... Don't send any state");
                return;
            }
        }
        if (agentMode == AgentMode::CLIENT) {
            ERROR_PRINT("Not applicable in CLIENT mode");
            return;
        }
        ensureConnected();
        DS5W::setDeviceOutputState(&controller, &outState);
    }

    void disable(void) {
        std::lock_guard<std::mutex> lock(enabledMutex);
        enabled = false;
    }

    void enable(void) {
        std::lock_guard<std::mutex> lock(enabledMutex);
        enabled = true;
    }

    bool isEnabled(void) {
        std::lock_guard<std::mutex> lock(enabledMutex);
        return enabled;
    }

    void reset(void) {
        dualsensitive::setLeftTrigger(TriggerProfile::Normal);
        dualsensitive::setRightTrigger(TriggerProfile::Normal);
    }
}
