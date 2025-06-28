
#include "Logger.h"
#include <filesystem>

namespace Logger {

    static std::atomic_bool debugEnabled = false;
    static std::ostream* logOut = &std::cout;
    static std::unique_ptr<std::ofstream> logFile;
    static std::mutex logMutex;

    void init(bool enableDebug) {
        debugEnabled.store(enableDebug);
    }

    bool isDebugEnabled() {
        return debugEnabled.load();
    }

    bool setLogFile(const std::string& filePath) {
        std::lock_guard<std::mutex> lock(logMutex);
        logFile = std::make_unique<std::ofstream> ( 
                filePath,
                std::ios::out | std::ios::app
        );
        if (logFile && logFile->is_open()) {
            logOut = logFile.get();
            return true;
        } else {
            logFile.reset();
            logOut = &std::cout;
            return false;
        }
    }

    void debugImpl(const std::string& message) {
        std::lock_guard<std::mutex> lock(logMutex);
        (*logOut) << "[DEBUG] " << message << std::endl;
    }

    void infoImpl(const std::string& message) {
        std::lock_guard<std::mutex> lock(logMutex);
        (*logOut) << "[INFO] " << message << std::endl;
    }

    void errorImpl(const std::string& message) {
        std::lock_guard<std::mutex> lock(logMutex);
        (*logOut) << "[ERROR] " << message << std::endl;
    }

}
