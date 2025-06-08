#include <Windows.h>

#include <string>
#include <sstream>

#include <IO.h>
#include <Device.h>
#include <Helpers.h>
#include <iostream>
#include <dualsense.h>

int main(int argc, char** argv){
    dualsense::init();
    dualsense::ensureConnected();
    std::cout << "is controller connected: " << dualsense::isConnected() << std::endl;

    // Weapon profiles
    // XXX NOTE: Those are created by me empirically, so they are not accurate

    std::cout << "mode changed to soft" << std::endl;
    dualsense::setLeftTrigger(TriggerProfile::Choppy);
    dualsense::setRightTrigger(TriggerProfile::Soft);
    dualsense::sendState();
    Sleep(4000);

    std::cout << "mode changed to shotgun" << std::endl;
    dualsense::setLeftCustomTrigger(TriggerMode::Rigid_A, {60, 71, 56, 128, 195, 210, 255});
    dualsense::setRightTrigger(TriggerProfile::SlopeFeedback, {0, 5, 1, 8});
    dualsense::sendState();
    Sleep(4000);
 
    std::cout << "mode changed to SMG" << std::endl;
    dualsense::setLeftCustomTrigger(TriggerMode::Rigid_A, {71, 96, 128, 128, 128, 128, 128} );
    dualsense::setRightTrigger(TriggerProfile::Vibration, {3, 4, 14} );
    dualsense::sendState();
    Sleep(4000);

    std::cout << "mode changed to railgun" << std::endl;
    dualsense::setLeftTrigger(TriggerProfile::Machine, {1, 8, 3, 3, 184, 0} );
    dualsense::setRightCustomTrigger(TriggerMode::Pulse_B, {238, 215, 66, 120, 43, 160, 215});
    dualsense::sendState();
    Sleep(4000);

    std::cout << "mode changed to rocket launcher (triple-shot)" << std::endl;
    dualsense::setLeftCustomTrigger(TriggerMode::Rigid, {} );
    dualsense::setRightCustomTrigger(TriggerMode::Rigid_A, {209, 42, 232, 192, 232, 209, 232} );
    dualsense::sendState();
    Sleep(4000);

    std::cout << "mode changed to sticky launcher" << std::endl;
    dualsense::setLeftTrigger(TriggerProfile::Feedback, {3, 3} );
    dualsense::setRightTrigger(TriggerProfile::VeryHard, {} );
    dualsense::sendState();
    Sleep(4000);

    std::cout << "mode changed to pistol (with silencer)" << std::endl;
    dualsense::setLeftTrigger(TriggerProfile::Choppy);
    dualsense::setRightTrigger(TriggerProfile::Weapon, {2, 5, 5});
    dualsense::sendState();
    Sleep(4000);

    std::cout << "mode changed to pistol" << std::endl;
    dualsense::setLeftTrigger(TriggerProfile::Choppy);
    dualsense::setRightTrigger(TriggerProfile::MultiplePositionFeeback, {4, 7, 0, 2, 4, 6, 0, 3, 6, 0});
    dualsense::sendState();
    Sleep(4000);

    std::cout << "mode changed to magnum" << std::endl;
    dualsense::setLeftTrigger(TriggerProfile::Choppy);
    dualsense::setRightTrigger(TriggerProfile::SlopeFeedback, {0, 8, 8, 1});
    dualsense::sendState();
    Sleep(4000);

    std::cout << "mode changed to Medium Machine Gun" << std::endl;
    dualsense::setLeftTrigger(TriggerProfile::Vibration, {1, 10, 8});
    dualsense::setRightTrigger(TriggerProfile::MultiplePositionVibration, {10, 0, 1, 2, 3, 4, 5, 6, 7, 8, 8});
    dualsense::sendState();
    Sleep(4000);

    dualsense::terminate();
    std::cout << "is controller connected: " << dualsense::isConnected() << std::endl;

    return 0;
}
