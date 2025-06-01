
#ifndef LOG_H
#define LOG_H

#include <iostream>
#include <sstream>

// Debug-only logging macro
#ifndef NDEBUG
    #define DEBUG_PRINT(...)                  \
        do {                                  \
            std::ostringstream oss;           \
            oss << __VA_ARGS__;               \
            std::ostream &out = std::cout;    \
            out << "[DEBUG] ";                \
            out << oss.str();                 \
            out << std::endl;                 \
        } while (0)
#else
    #define DEBUG_PRINT(...) do {} while (0)
#endif

// Always prints to stdout
#define INFO_PRINT(...)                       \
    do {                                      \
        std::ostringstream oss;               \
        oss << __VA_ARGS__;                   \
        std::ostream &out = std::cout;        \
        out << "[INFO] ";                     \
        out << oss.str();                     \
        out << std::endl;                     \
    } while (0)

// Always prints to stderr
#define ERROR_PRINT(...)                      \
    do {                                      \
        std::ostringstream oss;               \
        oss << __VA_ARGS__;                   \
        std::ostream &out = std::cerr;        \
        out << "[ERROR] ";                    \
        out << oss.str();                     \
        out << std::endl;                     \
    } while (0)

#endif // LOG_H
