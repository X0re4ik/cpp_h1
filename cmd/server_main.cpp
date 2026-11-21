#include "h1/application.hpp"
#include "h1/common.hpp"
#include "h1/db/db.hpp"
#include "h1/io_server/handlers/task_manager.hpp"
#include "h1/io_server/io_server.hpp"
#include "h1/io_server/router.hpp"
#include "h1/io_server/server_config.hpp"
#include "h1/local_cache/local_cache.hpp"
#include "h1/logger/logger.hpp"

#include <exception>
#include <iostream>
#include <string>
#include <utility>

namespace
{

void initLogger(bool verbose)
{
    using h1::log::initLoggerConfig;
    using h1::log::LoggerConfig;
    using h1::log::LoggerLevelEnum;

    const auto logLevel =
        verbose ? LoggerLevelEnum::DEBUG : LoggerLevelEnum::OFF;
    initLoggerConfig(LoggerConfig()
                         .addConsoleLog()
                         .setPattern("[%H:%M:%S.%f] [%^%l%$] [%t] [%s:%#] %v")
                         .setLogLevel(logLevel)
                         .setLogName(h1::projectName));
}

h1::io_server::APIRouter getAPIRouter(h1::db::IDBManager& dbManager,
                                      h1::lcache::ICalcLocalCache& localCache)
{
    using h1::io_server::APIRouter;
    using h1::io_server::makeTaskManagerHandler;

    auto apiRouter = APIRouter("/h1");
    apiRouter.registerHandler("POST", "/task",
                              makeTaskManagerHandler(dbManager, localCache));

    return apiRouter;
}

} // namespace

int main(int argc, char* argv[])
{
    try
    {
        const auto config = h1::io_server::ServerConfig::parse(argc, argv);
        initLogger(config.verbose);

        auto dbManager = h1::db::initDataBaseManager();
        auto localCache = h1::lcache::initCalcLocalCacheManager();
        h1::warmUpLocalCache(*localCache, *dbManager);

        auto apiRouter = getAPIRouter(*dbManager, *localCache);
        h1::io_server::IOServer server(config.host, std::to_string(config.port),
                                       std::move(apiRouter), config.threads);
        server.run();
    }
    catch (const h1::H1BaseException& exc)
    {
        std::cerr << exc.what() << '\n';
        return 1;
    }
    catch (const std::exception& exc)
    {
        std::cerr << "exception: " << exc.what() << '\n';
        return 1;
    }
    catch (...)
    {
        std::cerr << "Критическая ошибка работы сервера" << '\n';
        return 1;
    }

    return 0;
}
