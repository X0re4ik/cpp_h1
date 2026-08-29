
#ifndef H_H1_LOGGER_LOG_SINK_b0027471171f14722347b439829dab05
#define H_H1_LOGGER_LOG_SINK_b0027471171f14722347b439829dab05

#include "./code_location.hpp"
#include "./enums.hpp"
#include "./exceptions.hpp"
#include "h1/common.hpp"

#include <memory>
#include <optional>
#include <source_location>
#include <string>

namespace h1::log
{
class ILogSink
{
  public:
    // Constructors
    ILogSink() = default;

    // Copy constructor/operator=
    ILogSink(const ILogSink&) = delete;
    ILogSink& operator=(const ILogSink&) = delete;

    // Move constructor/operator=
    ILogSink(ILogSink&&) = delete;
    ILogSink& operator=(ILogSink&&) = delete;

    // Destructor
    virtual ~ILogSink() = default;

    virtual void trace(const String&, const CodeLocation&) const = 0;
    virtual void debug(const String&, const CodeLocation&) const = 0;
    virtual void info(const String&, const CodeLocation&) const = 0;
    virtual void warn(const String&, const CodeLocation&) const = 0;
    virtual void error(const String&, const CodeLocation&) const = 0;
};

} // namespace h1::log
#endif