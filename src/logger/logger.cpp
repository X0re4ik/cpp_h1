#include "spdlog/common.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

#include "h1/common.hpp"
#include "h1/logger/code_location.hpp"
#include "h1/logger/di.hpp"
#include "h1/logger/enums.hpp"
#include "h1/logger/log_sink.hpp"

#include <memory>

namespace h1::log
{

namespace
{
spdlog::source_loc convertToSourceLoc(const CodeLocation& codeLoc) H1_NOEXCEPT
{
    using spdlog::source_loc;
    auto sourceLog =
        source_loc(codeLoc.getFilenameIn().data(), codeLoc.getlineIn(),
                   codeLoc.getFunctionIn().data());
    return sourceLog;
}

spdlog::level::level_enum
    convertToLevelEnum(const LoggerLevelEnum& loggerEnum) H1_NOEXCEPT
{
    using spdlog::level::level_enum;
    switch (loggerEnum)
    {
        case LoggerLevelEnum::INFO:
        {
            return level_enum::info;
        }
        case LoggerLevelEnum::DEBUG:
        {
            return level_enum::debug;
        }
        case LoggerLevelEnum::WARN:
        {
            return level_enum::warn;
        }
        case LoggerLevelEnum::ERROR:
        {
            return level_enum::err;
        }
        case LoggerLevelEnum::TRACE:
        {
            return level_enum::trace;
        }
        case LoggerLevelEnum::OFF:
        {
            return level_enum::off;
        }
        case LoggerLevelEnum::CRITICAL:
        {
            return level_enum::critical;
        }
        default:
        {
            return level_enum::off;
        }
    }
}

} // namespace

// ===== [START] ProjectLogger =====
class ProjectLogger : public ILogSink
{
  public:
    // Aliases
    using LoggerSharedPtr = std::shared_ptr<spdlog::logger>;

    // Constructors
    explicit ProjectLogger(LoggerSharedPtr&& logger);

    // Methods
    void trace(const String& fmt, const CodeLocation& codeLoc) const override;
    void debug(const String& fmt, const CodeLocation& codeLoc) const override;
    void info(const String& fmt, const CodeLocation& codeLoc) const override;
    void warn(const String& fmt, const CodeLocation& codeLoc) const override;
    void error(const String& fmt, const CodeLocation& codeLoc) const override;

  private:
    LoggerSharedPtr logger_;
};

ProjectLogger::ProjectLogger(ProjectLogger::LoggerSharedPtr&& logger) :
    logger_(std::move(logger))
{}

void ProjectLogger::trace(const String& fmt, const CodeLocation& codeLoc) const
{
    using spdlog::level::level_enum;
    auto sourceLog = convertToSourceLoc(codeLoc);
    logger_->log(sourceLog, level_enum::trace, fmt);
}

void ProjectLogger::info(const String& fmt, const CodeLocation& codeLoc) const
{
    using spdlog::level::level_enum;
    auto sourceLog = convertToSourceLoc(codeLoc);
    logger_->log(sourceLog, level_enum::info, fmt);
}

void ProjectLogger::debug(const String& fmt, const CodeLocation& codeLoc) const
{
    using spdlog::level::level_enum;
    auto sourceLog = convertToSourceLoc(codeLoc);
    logger_->log(sourceLog, level_enum::debug, fmt);
}

void ProjectLogger::error(const String& fmt, const CodeLocation& codeLoc) const
{
    using spdlog::level::level_enum;
    auto sourceLog = convertToSourceLoc(codeLoc);
    logger_->log(sourceLog, level_enum::err, fmt);
}

void ProjectLogger::warn(const String& fmt, const CodeLocation& codeLoc) const
{
    using spdlog::level::level_enum;
    auto sourceLog = convertToSourceLoc(codeLoc);
    logger_->log(sourceLog, level_enum::warn, fmt);
}

// ===== [END] ProjectLogger =====

// ===== [START] ProjectLoggerFactory =====
class ProjectLoggerFactory
{

  public:
    using PtrProjectLogger = std::unique_ptr<ILogSink>;

    static PtrProjectLogger& getInstance(const LoggerConfig& config) H1_NOEXCEPT
    {
        static auto ptrLogger = createInstance(config);
        return ptrLogger;
    }

  private:
    static PtrProjectLogger
        createInstance(const LoggerConfig& config) H1_NOEXCEPT
    {

        std::vector<spdlog::sink_ptr> sinks;

        // Конфигурирования логгера
        // Для подключения нового или своего собсвтенного логгера использовать
        // https://github.com/gabime/spdlog/wiki/Sinks

        // Логирование в консоль
        if (config.useConsoleLog())
        {
            sinks.push_back(
                std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
        }

        // Логирование в файл

        if (config.useFileLog())
        {
            const std::size_t mgByte = 5;
            const std::size_t maxSize = mgByte * 1024 * 1024;
            const std::size_t maxFiles = 10;
            sinks.push_back(
                std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                    config.getFileLog(), maxSize, maxFiles));
        }
        // Создание логгера
        auto loggerPtr = std::make_unique<spdlog::logger>(
            config.getLogName(), sinks.begin(), sinks.end());

        loggerPtr->set_pattern(config.getPattern());
        loggerPtr->set_level(convertToLevelEnum(config.getLevel()));

        return std::make_unique<ProjectLogger>(std::move(loggerPtr));
    }
};
// ===== [END] ProjectLoggerFactory =====

// ===== [START] getProjectLogger =====
const std::unique_ptr<ILogSink>& getProjectLogger(const LoggerConfig& config)
{
    return ProjectLoggerFactory::getInstance(config);
}
// ===== [END] getProjectLogger =====

// ===== [START] LoggerConfigFactory =====
class LoggerConfigFactory
{
  public:
    static LoggerConfig& getInstance(const LoggerConfig& config) H1_NOEXCEPT
    {
        static auto initConfigLogger = config;
        return initConfigLogger;
    }
};
// ===== [END] LoggerConfigFactory =====

// ===== [START] initLoggerConfig =====
const LoggerConfig& initLoggerConfig(const LoggerConfig& config)
{
    return LoggerConfigFactory::getInstance(config);
}
// ===== [END] initLoggerConfig =====

// ===== [START] initLoggerConfig =====
const LoggerConfig& getLoggerConfig()
{
    return LoggerConfigFactory::getInstance({});
}
// ===== [END] getLoggerConfig =====

} // namespace h1::log