#include "h1/db/pg/pg_params.hpp"

namespace h1::db::pg
{

PGParams::PGParams()
{
    const int maxReserve = 100;
    values_.reserve(maxReserve);
    ptrs_.reserve(maxReserve);
}

PGParams& PGParams::addNull()
{
    values_.emplace_back("NULL");
    return *this;
}

H1_NODISCARD int PGParams::size() const H1_NOEXCEPT
{
    return static_cast<int>(values_.size());
}

H1_NODISCARD const char* const* PGParams::values() const H1_NOEXCEPT
{
    return ptrs_.empty() ? nullptr : ptrs_.data();
}

void PGParams::finalize()
{
    ptrs_.clear();
    ptrs_.reserve(values_.size());
    for (auto& variable : values_)
    {
        ptrs_.push_back(variable.c_str());
    }
}

String PGParams::convert(int value)
{
    return std::to_string(value);
}
String PGParams::convert(std::int64_t value)
{
    return std::to_string(value);
}
String PGParams::convert(double value)
{
    return std::to_string(value);
}
String PGParams::convert(bool value)
{
    return value ? "true" : "false";
}
String PGParams::convert(const String& value)
{
    return value;
}

} // namespace h1::db::pg
