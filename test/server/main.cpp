
#include <dualsense.h>
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    std::cout << "Starting DualSense UDP server...\n";

    auto status = dualsense::init(AgentMode::SERVER, 9999);
    if (status != dualsense::Status::Ok) {
        std::cerr << "Failed to initialize DualSense in SERVER mode\n";
        return 1;
    }

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    dualsense::terminate();
    return 0;
}
