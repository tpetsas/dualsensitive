/*
    dualsensitive.h is part of DualSensitive
    https://github.com/tpetsas/dualsensitive

    Contributors of this file:
    05.2025 Thanasis Petsas

    Licensed under the MIT License
*/

#ifndef DUALSENSE_H
#define DUALSENSE_H

#include <cstdint>
#include <vector>
#include <string>
#include <vector>
#include <functional>

/**
 * Defines the operating mode of the DualSensitive library.
 * - SOLO: Directly accesses and controls the DualSense controller.
 * - SERVER: Listens for UDP trigger effect commands and applies them.
 * - CLIENT: Sends trigger commands to a background server process.
 */
enum class AgentMode {
    SOLO,
    SERVER,
    CLIENT
};

/**
 * Defines the payload type to be sent to the DualSensitive Service
 * Supported types:
 *  - BIND: bind packet with PID
 *  - TRIGGER: trigger packet (existing behavior)
 */
enum class PayloadType : uint8_t {
    BIND,
    TRIGGER
};



enum class TriggerMode : uint8_t {
    Off = 0x00,

    // Basic modes
    Rigid = 0x01,
    Pulse = 0x02,

    // Extended Rigid modes
    Rigid_A = 0x21,   // 0x01 | 0x20
    Rigid_B = 0x05,   // 0x01 | 0x04
    Rigid_AB = 0x25,  // 0x01 | 0x20 | 0x04

    // Extended Pulse modes
    Pulse_A = 0x22,   // 0x02 | 0x20
    Pulse_B = 0x06,   // 0x02 | 0x04
    Pulse_A2 = 0x23,
    Pulse_B2 = 0x26,
    Pulse_AB = 0x27,

    // Special modes
    Calibration = 0xFC,
    Feedback = 0x21,
    Weapon = 0x25,
    Vibration = 0x26
};

// Custom trigger profiles
// sources:
// * DualsenseX: https://github.com/Paliverse/DualSenseX
// * DualsenseY: https://github.com/WujekFoliarz/DualSenseY-v2
enum class TriggerProfile : int8_t {
    Normal = 0,

    GameCube,

    VerySoft,
    Soft,
    Medium,
    Hard,
    VeryHard,
    Hardest,

    Rigid,
    Choppy,

    VibrateTrigger,
    VibrateTriggerPulse,

    Resistance,
    Galloping,

    Machine,
    Feedback,

    Vibration,
    VibrateTrigger10Hz,

    SlopeFeedback,

    MultiplePositionFeeback,
    MultiplePositionVibration,

    Bow,
    Weapon,
    SemiAutomaticGun,
    AutomaticGun,

    Custom,
};

void setTriggerProfile(unsigned char *buffer, TriggerProfile profile, std::vector<uint8_t> extras = {});


/**
 * DualSensitive interface supporting multiple runtime modes.
 */
namespace dualsensitive {

    enum class Status {
        Ok = 0, 
        InitFailed,
        NoControllersDetected
    };

    bool isConnected(void);

    uint32_t getClientPid(void);
    void sendPidToServer(void);

    void ensureConnected(void);

    /**
     * Initializes the DualSensitive interface in the specified mode.
     * @param mode        SOLO, SERVER, or CLIENT.
     * @param logPath     the path to the file that will keep the dualsensitive logs
     * @param enableDebug enable debug logging
     * @param port        UDP port used by SERVER/CLIENT mode (default: 28472).
     * @return 0 if the module was successfully initialized,
     *         or an error code otherwise (see enum class ErrorCode)
     */
    Status init(AgentMode mode = AgentMode::SOLO,
                    const std::string& logPath = "dualsensitive.log",
                    bool enableDebug = true, uint16_t port = 28472);

    /**
     * Initializes the dualsensitive module
     * @return 0 if the module was successfully terminated,
     *         or an error code otherwise (see enum class ErrorCode)
     */
    void terminate(void);

    /**
    * Sets an adaptive trigger mode to the left trigger (i.e., L2)
    * @param triggerProfile The mode to set for the adaptive trigger
    * @param extras (optional) Additional parameters required by the trigger
    */
    void setLeftTrigger(TriggerProfile triggerProfile, std::vector<uint8_t> extras = {});

    /**
    * Sets an adaptive trigger mode to the right trigger (i.e., R2)
    * @param   triggerProfile The mode to set for the adaptive trigger
    * @param   extras (optional) Additional parameters required by the trigger
    */
    void setRightTrigger(TriggerProfile triggerProfile, std::vector<uint8_t> extras = {});
    /**
     * Sets an custom adaptive trigger mode to the left trigger (i.e., L2)
     *
     * NOTE: This allows for more complex trigger configurations, including a
     * custom value mode and additional parameters
     *
     * @param customMode   The custom value mode for more detailed trigger
     * control
     * @param extras   Additional parameters required by the custom trigger
     * mode
     */
    void setLeftCustomTrigger(TriggerMode customMode,
                                        std::vector<uint8_t> extras);
    /**
     * Sets an custom adaptive trigger mode to the right trigger (i.e., R2)
     *
     * NOTE: This allows for more complex trigger configurations, including a
     * custom value mode and additional parameters
     *
     * @param customMode   The custom value mode for more detailed trigger
     * control
     * @param extras   Additional parameters required by the custom trigger
     * mode
     */
    void setRightCustomTrigger(TriggerMode customMode,
                                        std::vector<uint8_t> extras);
    void sendState(void);

    /**
     * Disables dualsensitive operations. When dualsensitive is disabled,
     * no settings or profiles will be sent to controller
     */
    void disable(void);

    /**
     * Enables dualsensitive operations. When dualsensitive is disabled,
     * no settings or profiles will be sent to controller
     */
    void enable(void);

    /**
     * Returns if dualsensitive operations are enabled
     */
    bool isEnabled(void);

    /**
     * Resets the controller's setting to the default state
     */
    void reset(void);

    // Other DualSense features
    // XXX The following are not supported yet
#if 0
    /**
     * Sets an RGB LEDs update to the current controller's payload
     * @param red     The red (R) component of the color
     * @param green   The green (G) component of the color
     * @param blue    The blue (B) component of the color
     * @param brightness    The brightness level of the LEDs
     */
    void setRGB(int red, int green, int blue, int brightness);


    /**
     * Sets a microphone LED update (mute/unmute) to the current controller's
     * payload
     * mode  The microphone LED configuration to apply
     */
    void setMicLED(int mode);
#endif
    // controllerIndex overloads
    // XXX The following are not supported yet
#if 0
    /**
    * Sets an adaptive trigger mode to the left trigger (i.e., L2)
    * @param controllerIndex    The index of the controller to receive the trigger
    * @param triggerMode The mode to set for the adaptive trigger
    * @param extras (optional) Additional parameters required by the trigger
    */
    void setLeftTrigger(int controllerIndex, TriggerMode triggerMode, std::vector<int> extras={});

    /**
    * Sets an adaptive trigger mode to the right trigger (i.e., R2)
    * @param controllerIndex    The index of the controller to receive the trigger
    * @param   triggerMode The mode to set for the adaptive trigger
    * @param   extras (optional) Additional parameters required by the trigger
    */
    void setRightTrigger(int controllerIndex, TriggerMode triggerMode, std::vector<int> extras={});

    /**
     * Sets an custom adaptive trigger mode to the left trigger (i.e., L2)
     *
     * NOTE: This allows for more complex trigger configurations, including a
     * custom value mode and additional parameters
     *
     * @param controllerIndex    The index of the controller to receive the trigger
     * @param customMode   The custom value mode for more detailed trigger
     * control
     * @param extras   Additional parameters required by the custom trigger
     * mode
     */
    void setLeftCustomTrigger(int controllerIndex, CustomTriggerValueMode customMode,
                                                    std::vector<int> extras);
    /**
     * Sets an custom adaptive trigger mode to the right trigger (i.e., R2)
     *
     * NOTE: This allows for more complex trigger configurations, including a
     * custom value mode and additional parameters
     * @param controllerIndex    The index of the controller to receive the trigger
     * @param customMode   The custom value mode for more detailed trigger
     * control
     * @param extras   Additional parameters required by the custom trigger
     * mode
     */
    void setRightCustomTrigger(int controllerIndex, CustomTriggerValueMode customMode,
                                                    std::vector<int> extras);
    /**
     * Sets an RGB LEDs update to the current controller's payload
     * @param controllerIndex    The index of the controller to receive the trigger
     * @param red     The red (R) component of the color
     * @param green   The green (G) component of the color
     * @param blue    The blue (B) component of the color
     * @param brightness    The brightness level of the LEDs
     */
    void setRGB(int controllerIndex, int red, int green, int blue, int brightness);

    /**
     * Sets a microphone LED update (mute/unmute) to the current controller's
     * payload
     * @param controllerIndex    The index of the controller to receive the trigger
     * mode  The microphone LED configuration to apply
     */
    void setMicLED(int controllerIndex, int mode);

    /**
     * Resets the controller's setting to user's defaults
     * @param controllerIndex    The index of the controller to receive the trigger
     */
     void reset(int controllerIndex);
#endif
}

#endif // DUALSENSE_H 
