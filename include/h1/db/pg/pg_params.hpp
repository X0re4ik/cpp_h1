#pragma once
#ifndef H_H1_DB_PG_PARAMS_5a11defb00ca78618a469986a89f5ac1
#define H_H1_DB_PG_PARAMS_5a11defb00ca78618a469986a89f5ac1

#include "h1/common.hpp"

#include <cstdint>
#include <vector>

namespace h1::db::pg
{

class PGParams
{

  public:
    explicit PGParams();

    template <typename T>
    PGParams& add(const T& value);

    PGParams& addNull();

    H1_NODISCARD int size() const H1_NOEXCEPT;

    H1_NODISCARD const char* const* values() const H1_NOEXCEPT;

    void finalize();

  private:
    static String convert(int value);
    static String convert(std::int64_t value);
    static String convert(double value);
    static String convert(bool value);
    static String convert(const String& value);

    std::vector<String> values_;
    std::vector<const char*> ptrs_;
};

template <typename T>
PGParams& PGParams::add(const T& value)
{
    values_.push_back(convert(value));
    return *this;
}

} // namespace h1::db::pg

#endif
