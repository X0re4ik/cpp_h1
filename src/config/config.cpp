#include "h1/config/config.hpp"

#include "h1/common.hpp"

#include <libenvpp/env.hpp>

#include <cstdint>
#include <string>

namespace h1::config
{

AppConfig AppConfig::fromEnv()
{
    auto hostExp = env::get<std::string>("DB_PGHOST");
    if (!hostExp.has_value())
    {
        throw std::runtime_error("DB_PGHOST: " + hostExp.error().what());
    }

    auto databaseExp = env::get<String>("DB_PGDATABASE");
    if (!databaseExp.has_value())
    {
        throw std::runtime_error("DB_PGDATABASE: " +
                                 databaseExp.error().what());
    }

    auto userExp = env::get<String>("DB_PGUSER");
    if (!userExp.has_value())
    {
        throw std::runtime_error("DB_PGUSER: " + userExp.error().what());
    }

    auto passwordExp = env::get<String>("DB_PGPASSWORD");
    if (!passwordExp.has_value())
    {
        throw std::runtime_error("DB_PGPASSWORD: " +
                                 passwordExp.error().what());
    }

    const auto port = env::get_or<std::uint16_t>("DB_PGPORT", 5432);

    return {
        .postgres =
            {
                .host = hostExp.value(),
                .port = port,
                .database = databaseExp.value(),
                .user = userExp.value(),
                .password = passwordExp.value(),
            },
    };
}

AppConfig getConfig()
{
    static auto config = AppConfig::fromEnv();
    return config;
}

} // namespace h1::config