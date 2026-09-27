#pragma once
#ifndef H_H1_DB_PG_EXCEPTIONS_cc293d19e501c1c34439064b9d4f3d42
#define H_H1_DB_PG_EXCEPTIONS_cc293d19e501c1c34439064b9d4f3d42

#include "h1/common.hpp"

#include <string>
#include <utility>

namespace h1::db::pg
{
class PGBaseException : public H1BaseException
{
  public:
    explicit PGBaseException(String msg) : H1BaseException(std::move(msg))
    {}
};

class PGConnectionError : public PGBaseException
{
  public:
    explicit PGConnectionError(const String& connectionTemplate) :
        PGBaseException("Error connect to PostgreSQL with : " +
                        connectionTemplate)
    {}
};

class PGConnectionNotOKException : public PGBaseException
{
  public:
    explicit PGConnectionNotOKException(String pgMsg) :
        PGBaseException(std::move(pgMsg))
    {}
};

class PGGetResultException : public PGBaseException
{
  public:
    explicit PGGetResultException(String pgMsg) :
        PGBaseException(std::move(pgMsg))
    {}
};

class InvalidPGResultException : public PGBaseException
{
  public:
    explicit InvalidPGResultException(String pgMsg, int statusCode) :
        PGBaseException(std::move(pgMsg) +
                        " Error Code: " + std::to_string(statusCode))
    {}
};

class ConvertToPGTypeException : public PGBaseException
{
  public:
    explicit ConvertToPGTypeException(const char* data,
                                      const std::string& typeName) :
        PGBaseException("Ошибка конвертации '" + std::string(data) +
                        "' в тип '" + typeName + "'")
    {}
};

} // namespace h1::db::pg

#endif
