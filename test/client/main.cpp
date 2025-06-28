
#include "dualsense.h"
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
    std::cout << "Client starting the DualSense Service process...\n";
    //if (!launchServer(serverProcInfo)) {
    if (!launchServerElevated()) {
        return 1;
    }

    std::this_thread::sleep_for(std::chrono::seconds(2)); // Give server time to start

    std::vector<uint8_t> payload;

    auto status = dualsense::init(AgentMode::CLIENT);
    if (status != dualsense::Status::Ok) {
        std::cout << "Failed to initialize Dualsensitive in CLIENT mode, status: " << static_cast<std::underlying_type<dualsense::Status>::type>(status) << std::endl;
        return 1;
    }

    std::cout << "mode changed to soft" << std::endl;
    dualsense::setLeftTrigger(TriggerProfile::Choppy);
    dualsense::setRightTrigger(TriggerProfile::Soft);
    Sleep(4000);

    std::cout << "mode changed to shotgun" << std::endl;
    dualsense::setLeftCustomTrigger(TriggerMode::Rigid_A, {60, 71, 56, 128, 195, 210, 255});
    dualsense::setRightTrigger(TriggerProfile::SlopeFeedback, {0, 5, 1, 8});
    Sleep(4000);

    //dualsense::terminate();
    dualsense::setLeftTrigger(TriggerProfile::Normal);
    dualsense::setRightTrigger(TriggerProfile::Normal);
    Sleep(4000);

    // Clean up
    if (terminateServer(serverProcInfo)) {
        std::cout << "Server terminated successfully.\n";
    } else {
        std::cerr << "Failed to terminate server.\n";
    }

    return 0;
}
