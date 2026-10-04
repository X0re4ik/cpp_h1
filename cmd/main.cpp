
#include "h1/application.hpp"
#include "h1/common.hpp"
#include "h1/db/db.hpp"
#include "h1/local_cache/local_cache.hpp"
#include "h1/logger/logger.hpp"

int main(int argc, char* argv[])
{
    auto dbManager = h1::db::initDataBaseManager();
    auto localCache = h1::lcache::initCalcLocalCacheManager();

    auto app = h1::Application(*dbManager, *localCache);
    auto code = app.run(argc, argv);
    return code;
}