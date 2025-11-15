#include "oracon/core/logger.h"
#include <iostream>
#include <ctime>
#include <iomanip>

namespace oracon {
namespace core {

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

void Logger::setLevel(LogLevel level) {
    m_level = level;
}

void Logger::log(LogLevel level, const String& message, const char* file, int line) {
    if (level < m_level) {
        return;
    }

    // Get current time
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);

    // Level string
    const char* levelStr = "";
    std::ostream* stream = &std::cout;

    switch (level) {
        case LogLevel::Debug:
            levelStr = "DEBUG";
            break;
        case LogLevel::Info:
            levelStr = "INFO";
            break;
        case LogLevel::Warning:
            levelStr = "WARN";
            stream = &std::cerr;
            break;
        case LogLevel::Error:
            levelStr = "ERROR";
            stream = &std::cerr;
            break;
        case LogLevel::Fatal:
            levelStr = "FATAL";
            stream = &std::cerr;
            break;
    }

    // Output format: [TIMESTAMP] [LEVEL] message
    *stream << "[" << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << "] "
            << "[" << levelStr << "] "
            << message;

    if (file && line > 0) {
        *stream << " (" << file << ":" << line << ")";
    }

    *stream << std::endl;

    // Flush on errors
    if (level >= LogLevel::Error) {
        stream->flush();
    }
}

} // namespace core
} // namespace oracon
