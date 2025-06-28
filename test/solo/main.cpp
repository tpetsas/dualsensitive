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
