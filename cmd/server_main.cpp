#include "h1/application.hpp"
#include "h1/common.hpp"
#include "h1/db/db.hpp"
#include "h1/io_server/handlers/task_manager.hpp"
#include "h1/io_server/io_server.hpp"
#include "h1/io_server/router.hpp"
#include "h1/local_cache/local_cache.hpp"
#include "h1/logger/logger.hpp"

#include <iostream>

void initLogger(bool verbose)
{

    using h1::log::initLoggerConfig;
    using h1::log::LoggerConfig;
    using h1::log::LoggerLevelEnum;
    const auto logLevel =
        verbose ? LoggerLevelEnum::DEBUG : LoggerLevelEnum::OFF;
    initLoggerConfig(LoggerConfig().addConsoleLog().setPattern(
        "[%H:%M:%S.%f] [%^%l%$] [%t] [%s:%#] %v"));
}

h1::io_server::APIRouter getAPIRouter(h1::db::IDBManager& dbManager,
                                      h1::lcache::ICalcLocalCache& localCache)
{

    auto apiRouter = h1::io_server::APIRouter("/h1");
    apiRouter.registerHandler(
        "POST", "/task/",
        h1::io_server::makeTaskManagerHandler(dbManager, localCache));

    return apiRouter;
}

int main(int argc, char* argv[])
{
    initLogger(true);
    auto dbManager = h1::db::initDataBaseManager();
    auto localCache = h1::lcache::initCalcLocalCacheManager();

    auto apiRouter = getAPIRouter(*dbManager, *localCache);
    try
    {
        h1::io_server::IOServer server("0.0.0.0", "8080", apiRouter, 4);
        server.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "exception: " << e.what() << "\n";
    }

    return 0;
}