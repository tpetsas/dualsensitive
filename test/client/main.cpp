
#include "dualsensitive.h"
#include "udp.h"
#include <windows.h>
#include <iostream>


bool launchServerElevated(const std::wstring& exePath = L"./dualsensitive-service.exe") {
    wchar_t fullExePath[MAX_PATH];
    if (!GetFullPathNameW(exePath.c_str(), MAX_PATH, fullExePath, nullptr)) {
        return false;
    }

    SHELLEXECUTEINFOW sei = { sizeof(sei) };
    sei.lpVerb = L"runas";
    sei.lpFile = fullExePath;
    sei.nShow = SW_HIDE;
    sei.fMask = SEE_MASK_NO_CONSOLE;

    if (!ShellExecuteExW(&sei)) {
        DWORD err = GetLastError();
        return false;
    }

    return true;
}

bool launchServer(PROCESS_INFORMATION& outProcInfo, const std::wstring& exePath = L"./dualsensitive-service.exe") {
    STARTUPINFOW si = { sizeof(si) };
    ZeroMemory(&outProcInfo, sizeof(outProcInfo));

    BOOL success = CreateProcessW(
        exePath.c_str(),
        nullptr,
        nullptr,
        nullptr,
        FALSE,
        0,
        nullptr,
        nullptr,
        &si,
        &outProcInfo
    );

    if (!success) {
        std::cerr << "Failed to launch dualsensitive-service.exe. Error: " << GetLastError() << "\n";
        return false;
    }

    // You can close thread handle immediately; we keep process handle
    CloseHandle(outProcInfo.hThread);
    return true;
}


bool terminateServer(PROCESS_INFORMATION& procInfo) {
    if (procInfo.hProcess != nullptr) {
        BOOL result = TerminateProcess(procInfo.hProcess, 0); // 0 = exit code
        CloseHandle(procInfo.hProcess);
        procInfo.hProcess = nullptr;
        return result == TRUE;
    }
    return false;
}

int main() {
    PROCESS_INFORMATION serverProcInfo;
    std::cout << "Client starting the DualSensitive Service process...\n";
    //if (!launchServer(serverProcInfo)) {
    if (!launchServerElevated()) {
        return 1;
    }

    std::this_thread::sleep_for(std::chrono::seconds(2)); // Give server time to start

    std::vector<uint8_t> payload;

    auto status = dualsensitive::init(AgentMode::CLIENT);
    if (status != dualsensitive::Status::Ok) {
        std::cout << "Failed to initialize DualSensitive in CLIENT mode, status: " << static_cast<std::underlying_type<dualsensitive::Status>::type>(status) << std::endl;
        return 1;
    }

    std::cout << "mode changed to soft" << std::endl;
    dualsensitive::setLeftTrigger(TriggerProfile::Choppy);
    dualsensitive::setRightTrigger(TriggerProfile::Soft);
    Sleep(4000);

    std::cout << "mode changed to shotgun" << std::endl;
    dualsensitive::setLeftCustomTrigger(TriggerMode::Rigid_A, {60, 71, 56, 128, 195, 210, 255});
    dualsensitive::setRightTrigger(TriggerProfile::SlopeFeedback, {0, 5, 1, 8});
    Sleep(4000);

    std::cout << "mode changed to SMG" << std::endl;
    dualsensitive::setLeftCustomTrigger(TriggerMode::Rigid_A, {71, 96, 128, 128, 128, 128, 128} );
    dualsensitive::setRightTrigger(TriggerProfile::Vibration, {3, 4, 14} );
    Sleep(4000);

    std::cout << "mode changed to railgun" << std::endl;
    dualsensitive::setLeftTrigger(TriggerProfile::Machine, {1, 8, 3, 3, 184, 0} );
    dualsensitive::setRightCustomTrigger(TriggerMode::Pulse_B, {238, 215, 66, 120, 43, 160, 215});
    Sleep(4000);

    std::cout << "mode changed to rocket launcher (triple-shot)" << std::endl;
    dualsensitive::setLeftCustomTrigger(TriggerMode::Rigid, {} );
    dualsensitive::setRightCustomTrigger(TriggerMode::Rigid_A, {209, 42, 232, 192, 232, 209, 232} );
    Sleep(4000);

    std::cout << "mode changed to sticky launcher" << std::endl;
    dualsensitive::setLeftTrigger(TriggerProfile::Feedback, {3, 3} );
    dualsensitive::setRightTrigger(TriggerProfile::VeryHard, {} );
    Sleep(4000);

    std::cout << "mode changed to pistol (with silencer)" << std::endl;
    dualsensitive::setLeftTrigger(TriggerProfile::Choppy);
    dualsensitive::setRightTrigger(TriggerProfile::Weapon, {2, 5, 5});
    Sleep(4000);

    std::cout << "mode changed to pistol" << std::endl;
    dualsensitive::setLeftTrigger(TriggerProfile::Choppy);
    dualsensitive::setRightTrigger(TriggerProfile::MultiplePositionFeeback, {4, 7, 0, 2, 4, 6, 0, 3, 6, 0});
    Sleep(4000);

    std::cout << "mode changed to magnum" << std::endl;
    dualsensitive::setLeftTrigger(TriggerProfile::Choppy);
    dualsensitive::setRightTrigger(TriggerProfile::SlopeFeedback, {0, 8, 8, 1});
    Sleep(4000);

    std::cout << "mode changed to Medium Machine Gun" << std::endl;
    dualsensitive::setLeftTrigger(TriggerProfile::Vibration, {1, 10, 8});
    dualsensitive::setRightTrigger(TriggerProfile::MultiplePositionVibration, {10, 0, 1, 2, 3, 4, 5, 6, 7, 8, 8});
    Sleep(4000);

    // Clean up
    if (terminateServer(serverProcInfo)) {
        std::cout << "DualSensitive Service terminated successfully.\n";
    } else {
        std::cerr << "Failed to terminate DualSensitive Service.\n";
    }

    return 0;
}
