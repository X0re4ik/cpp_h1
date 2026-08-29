#ifndef H_H1_LOGGER_CONFIG_5a257becbb7fede5e2477387768ad918
#define H_H1_LOGGER_CONFIG_5a257becbb7fede5e2477387768ad918

#include "./enums.hpp"
#include "./exceptions.hpp"
#include "h1/common.hpp"

#include <memory>
#include <optional>
#include <source_location>
#include <string>

namespace h1::log
{

class LoggerConfig
{

  public:
    // Constructors
    LoggerConfig() :
        logName_("project-logger"), logFilePath_(std::nullopt),
        logPattern_("%v")
    {}

    // Getters
    H1_NODISCARD const h1::String& getLogName() const H1_NOEXCEPT
    {
        return logName_;
    }
    H1_NODISCARD const h1::String& getPattern() const H1_NOEXCEPT
    {
        return logPattern_;
    }
    H1_NODISCARD bool useConsoleLog() const H1_NOEXCEPT
    {
        return useConsoleLog_;
    }
    H1_NODISCARD bool useFileLog() const H1_NOEXCEPT
    {
        return useFileLog_;
    }

    H1_NODISCARD const h1::String& getFileLog() const H1_EXCEPT
    {
        if (!logFilePath_.has_value())
        {
            throw LogFileNotFound();
        }
        return logFilePath_.value();
    }

    H1_NODISCARD LoggerLevelEnum getLevel() const H1_NOEXCEPT
    {
        return logLevel_;
    }

    // Setters
    LoggerConfig& setPattern(String pattern) H1_NOEXCEPT
    {
        logPattern_ = std::move(pattern);
        return *this;
    }
    LoggerConfig& setLogName(String logName) H1_NOEXCEPT
    {
        logName_ = std::move(logName);
        return *this;
    }
    LoggerConfig& setLogLevel(LoggerLevelEnum logLevel) H1_NOEXCEPT
    {
        logLevel_ = logLevel;
        return *this;
    }

    // Add Sinks
    LoggerConfig& addConsoleLog() H1_NOEXCEPT
    {
        useConsoleLog_ = true;
        return *this;
    }
    LoggerConfig& addFileLog(String path) H1_NOEXCEPT
    {
        logFilePath_ = std::move(path);
        useFileLog_ = true;
        return *this;
    }

  private:
    bool useConsoleLog_ = false;
    bool useFileLog_ = false;
    LoggerLevelEnum logLevel_ = LoggerLevelEnum::DEBUG;

    String logName_;
    std::optional<String> logFilePath_;
    String logPattern_;
};

} // namespace h1::log

#endif