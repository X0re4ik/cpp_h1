
#include "h1/logger/logger.hpp"

void initProjectLogger()
{

    auto logger =
        h1::log::LoggerConfig().addConsoleLog().setLogName("IUCH").setPattern(
            "[%#] %v");
    h1::log::initLoggerConfig(logger);
}

int main(int /*argc*/, char** /*argv*/)
{
    initProjectLogger();
    H1_LOG_INFO("Init Project Logger1");
    return 0;
}
