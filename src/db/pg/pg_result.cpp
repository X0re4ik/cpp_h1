#include "h1/db/pg/pg_result.hpp"

#include "h1/db/pg/pg_exceptions.hpp"

#include <cstdlib>
#include <cstring>

namespace h1::db::pg
{

int ConvertPGCharTo<int>::convert(const char* data)
{
    try
    {
        return std::atoi(data);
    }
    catch (const std::exception&)
    {
        throw ConvertToPGTypeException(data, "int");
    }
}

double ConvertPGCharTo<double>::convert(const char* data)
{
    try
    {
        return std::atof(data);
    }
    catch (const std::exception&)
    {
        throw ConvertToPGTypeException(data, "double");
    }
}

std::string ConvertPGCharTo<std::string>::convert(const char* data)
{
    try
    {
        return std::string{data};
    }
    catch (const std::exception&)
    {
        throw ConvertToPGTypeException(data, "string");
    }
}

std::int64_t ConvertPGCharTo<std::int64_t>::convert(const char* data)
{
    try
    {
        return std::stoll(data);
    }
    catch (const std::exception&)
    {
        throw ConvertToPGTypeException(data, "int64_t");
    }
}

bool ConvertPGCharTo<bool>::convert(const char* data)
{
    try
    {
        // NOLINTNEXTLINE (cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return data != nullptr && data[0] == 't';
    }
    catch (const std::exception&)
    {
        throw ConvertToPGTypeException(data, "bool");
    }
}

// NOLINTNEXTLINE (readability-named-parameter)
std::nullopt_t ConvertPGCharTo<std::nullopt_t>::convert(const char*)
{
    return std::nullopt;
}

bool PGRawResult::isNull(const char* data)
{
    const char* null = "NULL";
    return data == nullptr || std::strcmp(data, null) == 0;
}

void PGRawResult::PGResultDeleter::operator()(PGresult* pgResult) const
    H1_NOEXCEPT
{
    if (pgResult != nullptr)
    {
        PQclear(pgResult);
    }
}

PGRawResult::PGRawResult(PGresult* result) : result_(result)
{

    auto* rawResult = result_.get();
    if (rawResult == nullptr)
    {
        throw PGGetResultException("Ошибка получения данных");
    }
}

H1_NODISCARD PGRawResult::Index_t
    PGRawResult::columnIndex(const char* name) const H1_NOEXCEPT
{
    return PQfnumber(result_.get(), name);
}

H1_NODISCARD bool PGRawResult::ok() const H1_NOEXCEPT
{
    if (result_ == nullptr)
    {
        return false;
    }

    auto status = getStatus();
    return status == PGRES_TUPLES_OK || status == PGRES_COMMAND_OK;
}

H1_NODISCARD int PGRawResult::getStatus() const H1_NOEXCEPT
{
    return PQresultStatus(result_.get());
}

H1_NODISCARD PGRawResult::Index_t PGRawResult::rowsCount() const H1_NOEXCEPT
{
    return PQntuples(result_.get());
}

H1_NODISCARD PGRawResult::Index_t PGRawResult::rowsColumns() const H1_NOEXCEPT
{
    return PQnfields(result_.get());
}

H1_NODISCARD const char*
    PGRawResult::value(PGRawResult::Index_t row,
                       PGRawResult::Index_t column) const H1_NOEXCEPT
{
    return PQgetvalue(result_.get(), row, column);
}

} // namespace h1::db::pg
