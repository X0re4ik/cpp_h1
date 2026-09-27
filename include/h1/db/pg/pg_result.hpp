#pragma once
#ifndef H_H1_DB_PG_RESULT_03bac1b1027c61c1d62044cef31dc72c
#define H_H1_DB_PG_RESULT_03bac1b1027c61c1d62044cef31dc72c

#include "./pg_exceptions.hpp"
#include "h1/common.hpp"

#include <concepts>
#include <cstdint>
#include <libpq-fe.h>

#include <memory>
#include <optional>
#include <string>

namespace h1::db::pg
{

template <typename T>
class ConvertPGCharTo
{
  public:
    static int convert(const char* data);
};

template <typename T>
concept PGCovertable = std::same_as<T, int> || std::same_as<T, double> ||
    std::same_as<T, std::string> || std::same_as<T, std::int64_t> ||
    std::same_as<T, bool>;

class PGRawResult
{
  public:
    struct PGResultDeleter
    {
        void operator()(PGresult* pgResult) const H1_NOEXCEPT;
    };

    using Index_t = int;
    using PGResPtr = std::unique_ptr<PGresult, PGResultDeleter>;

    explicit PGRawResult(PGresult* result);

    H1_NODISCARD Index_t columnIndex(const char* name) const H1_NOEXCEPT;
    H1_NODISCARD bool ok() const H1_NOEXCEPT;
    H1_NODISCARD int getStatus() const H1_NOEXCEPT;
    H1_NODISCARD Index_t rowsCount() const H1_NOEXCEPT;
    H1_NODISCARD Index_t rowsColumns() const H1_NOEXCEPT;
    H1_NODISCARD const char* value(Index_t row,
                                   Index_t column) const H1_NOEXCEPT;
    template <PGCovertable T>
    std::optional<T> get(const String& column, Index_t index) const;

  private:
    PGResPtr result_;

    H1_NODISCARD static bool isNull(const char* data);
};

template <>
class ConvertPGCharTo<int>
{
  public:
    static int convert(const char* data);
};

template <>
class ConvertPGCharTo<double>
{
  public:
    static double convert(const char* data);
};

template <>
class ConvertPGCharTo<std::string>
{
  public:
    static std::string convert(const char* data);
};

template <>
class ConvertPGCharTo<std::int64_t>
{
  public:
    static std::int64_t convert(const char* data);
};

template <>
class ConvertPGCharTo<bool>
{
  public:
    static bool convert(const char* data);
};

template <>
class ConvertPGCharTo<std::nullopt_t>
{
  public:
    static std::nullopt_t convert(const char* data);
};

template <PGCovertable T>
std::optional<T> PGRawResult::get(const String& column, Index_t index) const
{
    auto cIndex = columnIndex(column.c_str());
    const auto* charValue = value(index, cIndex);
    if (charValue == nullptr || isNull(charValue))
    {
        return ConvertPGCharTo<std::nullopt_t>::convert(charValue);
    }
    return ConvertPGCharTo<T>::convert(charValue);
}

} // namespace h1::db::pg

#endif
