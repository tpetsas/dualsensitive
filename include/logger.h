
#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <atomic>
#include <memory>
#include <mutex>

namespace Logger {

    void init(bool enableDebug);
    bool isDebugEnabled();

    // Set log file path (call before any log output)
    bool setLogFile(const std::string& filePath); // returns success

    void debugImpl(const std::string& message);
    void infoImpl(const std::string& message);
    void errorImpl(const std::string& message);

} // namespace Logger

// Macro stream interface
#define DEBUG_PRINT(expr)                                      \
    do {                                                       \
        if (Logger::isDebugEnabled()) {                        \
            std::ostringstream oss__;                          \
            oss__ << expr;                                     \
            Logger::debugImpl(oss__.str());                    \
        }                                                      \
    } while (0)

#define INFO_PRINT(expr)                                       \
    do {                                                       \
        std::ostringstream oss__;                              \
        oss__ << expr;                                         \
        Logger::infoImpl(oss__.str());                         \
    } while (0)

#define ERROR_PRINT(expr)                                      \
    do {                                                       \
        std::ostringstream oss__;                              \
        oss__ << expr;                                         \
        Logger::errorImpl(oss__.str());                        \
    } while (0)

#endif // LOGGER_H
