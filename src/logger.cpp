#include "h1/logger.hpp"

#include "spdlog/common.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

namespace h1_logger
{

namespace
{
spdlog::source_loc convertToSourceLoc(const CodeLocation& codeLoc)
{
    using spdlog::source_loc;
    auto sourceLog =
        source_loc(codeLoc.getFilenameIn().data(), codeLoc.getlineIn(),
                   codeLoc.getFunctionIn().data());
    return sourceLog;
}
} // namespace

// ===== ProjectLogger =====
class ProjectLogger : public ILogSink
{
  public:
    // Aliases
    using LoggerSharedPtr = std::shared_ptr<spdlog::logger>;

    // Constructors
    explicit ProjectLogger(LoggerSharedPtr&& logger);

    // Methods
    void trace(const fString& fmt, const CodeLocation& codeLoc) const override;
    void debug(const fString& fmt, const CodeLocation& codeLoc) const override;
    void info(const fString& fmt, const CodeLocation& codeLoc) const override;
    void warn(const fString& fmt, const CodeLocation& codeLoc) const override;
    void error(const fString& fmt, const CodeLocation& codeLoc) const override;
    void setLevel(const LoggerLevelEnum& level) const override;

  private:
    LoggerSharedPtr logger_;
};

ProjectLogger::ProjectLogger(ProjectLogger::LoggerSharedPtr&& logger) :
    logger_(std::move(logger))
{}

void ProjectLogger::trace(const ProjectLogger::fString& fmt,
                          const CodeLocation& codeLoc) const
{
    using spdlog::level::level_enum;
    auto sourceLog = convertToSourceLoc(codeLoc);
    logger_->log(sourceLog, level_enum::trace, fmt);
}

void ProjectLogger::info(const ProjectLogger::fString& fmt,
                         const CodeLocation& codeLoc) const
{
    using spdlog::level::level_enum;
    auto sourceLog = convertToSourceLoc(codeLoc);
    logger_->log(sourceLog, level_enum::info, fmt);
}

void ProjectLogger::debug(const ProjectLogger::fString& fmt,
                          const CodeLocation& codeLoc) const
{
    using spdlog::level::level_enum;
    auto sourceLog = convertToSourceLoc(codeLoc);
    logger_->log(sourceLog, level_enum::debug, fmt);
}

void ProjectLogger::error(const ProjectLogger::fString& fmt,
                          const CodeLocation& codeLoc) const
{
    using spdlog::level::level_enum;
    auto sourceLog = convertToSourceLoc(codeLoc);
    logger_->log(sourceLog, level_enum::err, fmt);
}

void ProjectLogger::warn(const ProjectLogger::fString& fmt,
                         const CodeLocation& codeLoc) const
{
    using spdlog::level::level_enum;
    auto sourceLog = convertToSourceLoc(codeLoc);
    logger_->log(sourceLog, level_enum::warn, fmt);
}

void ProjectLogger::setLevel(const LoggerLevelEnum& level) const
{
    using spdlog::level::level_enum;
    logger_->set_level(static_cast<level_enum>(level));
}
// ===== END =====

// ===== ProjectLoggerFactory =====

const ProjectLoggerFactory::PtrProjectLogger&
    ProjectLoggerFactory::getInstance()
{
    static auto ptrLogger = createInstance();
    return ptrLogger;
}

ProjectLoggerFactory::PtrProjectLogger ProjectLoggerFactory::createInstance()
{

    std::vector<spdlog::sink_ptr> sinks;

    // Конфигурирования логгера
    // Для подключения нового или своего собсвтенного логгера использовать
    // https://github.com/gabime/spdlog/wiki/Sinks

    // Логирование в консоль
    sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());

    // Логирование в файл
    const std::size_t mgByte = 5;
    const std::size_t maxSize = mgByte * 1024 * 1024;
    const std::size_t maxFiles = 10;
    sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        "rotating_file_sink_mt_log", maxSize, maxFiles));

    // Создание логгера
    auto loggerPtr = std::make_shared<spdlog::logger>(
        "project-logger", sinks.begin(), sinks.end());

    loggerPtr->set_pattern(
        "[%H:%M:%S %z] [%n] [%^---%L---%$] [thread %t] [%s:%# %!] %v");

    return std::make_shared<ProjectLogger>(std::move(loggerPtr));
}
// ===== END =====

} // namespace h1_logger