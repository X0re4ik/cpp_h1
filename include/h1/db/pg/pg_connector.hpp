#pragma once
#ifndef H_H1_DB_PG_CONNECTOR_e4196330a1fd1e3eb7166a094fb79c2a
#define H_H1_DB_PG_CONNECTOR_e4196330a1fd1e3eb7166a094fb79c2a

#include "./pg_params.hpp"
#include "./pg_result.hpp"
#include "h1/common.hpp"

#include <libpq-fe.h>

#include <memory>

namespace h1::db::pg
{

class PGConnector
{
  public:
    struct PGconnDeleter
    {
        void operator()(PGconn* conn) const H1_NOEXCEPT;
    };
    using PGconnPtr = std::unique_ptr<PGconn, PGconnDeleter>;

    explicit PGConnector(String pgConnection);

    PGRawResult execute(const String& sql) H1_EXCEPT;

    PGRawResult executeOrError(const String& sql) H1_EXCEPT;

    PGRawResult executeParamsOrError(const String& sql,
                                     const PGParams& params) H1_EXCEPT;

  private:
    PGconnPtr pgConn_;
};

using PGConnectorPtr = std::unique_ptr<PGConnector>;

PGConnectorPtr makePGConnector(const String& pgConnection);

} // namespace h1::db::pg

#endif
