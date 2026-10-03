#include "h1/db/pg/pg_connector.hpp"

#include "h1/config/config.hpp"
#include "h1/db/pg/pg_exceptions.hpp"

#include <algorithm>
#include <exception>
#include <iostream>
#include <sstream>

namespace h1::db::pg
{

void PGConnector::PGconnDeleter::operator()(PGconn* conn) const H1_NOEXCEPT
{
    if (conn != nullptr)
    {
        PQfinish(conn);
    }
}

PGConnector::PGConnector(String pgConnection) :
    pgConn_(PQconnectdb(pgConnection.data()))
{
    auto* conn = pgConn_.get();

    if (conn == nullptr)
    {
        throw PGConnectionError(pgConnection);
    }

    if (PQstatus(conn) != CONNECTION_OK)
    {
        String error = PQerrorMessage(conn);
        throw PGConnectionNotOKException(error);
    }
}

PGRawResult PGConnector::execute(const String& sql)
{
    PGresult* rawResult = PQexec(pgConn_.get(), sql.data());
    return PGRawResult{rawResult};
}

PGRawResult PGConnector::executeOrError(const String& sql)
{
    auto res = execute(sql);
    if (!res.ok())
    {
        throw InvalidPGResultException("Ошибка выполнения запроса",
                                       res.getStatus());
    }
    return res;
}

PGRawResult PGConnector::executeParamsOrError(const String& sql,
                                              const PGParams& params)
{
    PGresult* rawResult =
        PQexecParams(pgConn_.get(), sql.c_str(), params.size(), nullptr,
                     params.values(), nullptr, nullptr, 0);
    auto res = PGRawResult{rawResult};

    if (!res.ok())
    {
        throw InvalidPGResultException("Ошибка выполнения запроса",
                                       res.getStatus());
    }

    return res;
}

PGConnectorPtr makePGConnector(const String& pgConnection)
{
    try
    {
        return std::make_unique<PGConnector>(pgConnection);
    }
    catch (const std::exception& e)
    {
        throw PGBaseException(e.what());
    }
}

PGConnectorPtr makePGConnector()
{
    auto config = h1::config::getConfig();
    auto libpqConnect = config.postgres.getlibpq();
    return makePGConnector(libpqConnect);
}

} // namespace h1::db::pg
