#ifndef H_H1_LOGGER_DI_9eb2c03b483a75f430d290ed89181c2f
#define H_H1_LOGGER_DI_9eb2c03b483a75f430d290ed89181c2f

#include "./code_location.hpp"
#include "./config.hpp"
#include "./log_sink.hpp"

namespace h1::log
{

const std::unique_ptr<ILogSink>& getProjectLogger(const LoggerConfig& config);

const LoggerConfig& getLoggerConfig();
const LoggerConfig& initLoggerConfig(const LoggerConfig& config);
} // namespace h1::log

#endif