#ifndef ORACON_CORE_LOGGER_H
#define ORACON_CORE_LOGGER_H

#include "types.h"
#include <string>
#include <iostream>
#include <sstream>

namespace oracon {
namespace core {

enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error,
    Fatal
};

class Logger {
public:
    static Logger& getInstance();

    void setLevel(LogLevel level);
    LogLevel getLevel() const { return m_level; }

    void log(LogLevel level, const String& message, const char* file = nullptr, int line = 0);

    template<typename... Args>
    void debug(Args&&... args) {
        if (m_level <= LogLevel::Debug) {
            log(LogLevel::Debug, format(std::forward<Args>(args)...));
        }
    }

    template<typename... Args>
    void info(Args&&... args) {
        if (m_level <= LogLevel::Info) {
            log(LogLevel::Info, format(std::forward<Args>(args)...));
        }
    }

    template<typename... Args>
    void warning(Args&&... args) {
        if (m_level <= LogLevel::Warning) {
            log(LogLevel::Warning, format(std::forward<Args>(args)...));
        }
    }

    template<typename... Args>
    void error(Args&&... args) {
        if (m_level <= LogLevel::Error) {
            log(LogLevel::Error, format(std::forward<Args>(args)...));
        }
    }

    template<typename... Args>
    void fatal(Args&&... args) {
        log(LogLevel::Fatal, format(std::forward<Args>(args)...));
    }

private:
    Logger() : m_level(LogLevel::Info) {}

    template<typename... Args>
    String format(Args&&... args) {
        std::ostringstream oss;
        (oss << ... << std::forward<Args>(args));
        return oss.str();
    }

    LogLevel m_level;
};

// Convenience macros
#define ORACON_LOG_DEBUG(...) ::oracon::core::Logger::getInstance().debug(__VA_ARGS__)
#define ORACON_LOG_INFO(...) ::oracon::core::Logger::getInstance().info(__VA_ARGS__)
#define ORACON_LOG_WARNING(...) ::oracon::core::Logger::getInstance().warning(__VA_ARGS__)
#define ORACON_LOG_ERROR(...) ::oracon::core::Logger::getInstance().error(__VA_ARGS__)
#define ORACON_LOG_FATAL(...) ::oracon::core::Logger::getInstance().fatal(__VA_ARGS__)

} // namespace core
} // namespace oracon

#endif // ORACON_CORE_LOGGER_H
