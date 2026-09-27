#pragma once
#ifndef H_H1_DB_PG_CONFIG_d29ba794f620fe5e2b83e123584cd520
#define H_H1_DB_PG_CONFIG_d29ba794f620fe5e2b83e123584cd520

#include "h1/common.hpp"

namespace h1::db::pg
{
struct PGConfig
{
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    String host;
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    int port;
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    String dbname;
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    String user;
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    String password;
};

} // namespace h1::db::pg

#endif
