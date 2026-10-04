#pragma once
#ifndef H_H1_CONFIG_49157b84a9d0d0e185661e69a0b2f25f
#define H_H1_CONFIG_49157b84a9d0d0e185661e69a0b2f25f

#include "h1/common.hpp"

#include <cstdint>
#include <sstream>
#include <string>

namespace h1::config
{

struct PostgresConfig
{

    // NOLINTNEXTLINE (misc-non-private-member-variables-in-classes)
    String host;
    // NOLINTNEXTLINE (misc-non-private-member-variables-in-classes)
    std::uint16_t port;
    // NOLINTNEXTLINE (misc-non-private-member-variables-in-classes)
    String database;
    // NOLINTNEXTLINE (misc-non-private-member-variables-in-classes)
    String user;
    // NOLINTNEXTLINE (misc-non-private-member-variables-in-classes)
    String password;

    H1_NODISCARD String getlibpq() const
    {
        std::stringstream sstream;
        sstream << "host=" << host << " port=" << port << " dbname=" << database
                << " user=" << user << " password=" << password;
        return sstream.str();
    }
};

struct AppConfig
{
    PostgresConfig postgres;
    static AppConfig fromEnv();
};

AppConfig getConfig();

} // namespace h1::config

#endif