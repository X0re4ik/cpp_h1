#ifndef H_H1_LOGGER_ENUMS_7d6c6d3372f9567e1eb36ec66e15b988
#define H_H1_LOGGER_ENUMS_7d6c6d3372f9567e1eb36ec66e15b988

#include <sys/types.h>
namespace h1::log
{
enum class LoggerLevelEnum : u_int8_t
{
    TRACE = 0,
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERROR = 4,
    CRITICAL = 5,
    OFF = 6
};

} // namespace h1::log

#endif
