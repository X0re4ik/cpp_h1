#pragma once
#ifndef H_H1_ENTITY_CALCULATION_RESULT_c99fc7291c4cf2de732c34d3c6a9577b
#define H_H1_ENTITY_CALCULATION_RESULT_c99fc7291c4cf2de732c34d3c6a9577b

#include "h1/common.hpp"

#include <cstdint>
#include <optional>
#include <sstream>
#include <string>

namespace h1::entity
{

struct CalculationResultEntity
{
    using Value_t = double;

    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    std::int64_t id;

    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    Value_t left;

    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    Value_t right;

    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    std::optional<Value_t> result;

    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    String operation;

    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    int statusCode;

    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    std::optional<String> errorMessage;

    H1_NODISCARD String toString() const H1_EXCEPT
    {
        std::string resultStr =
            result.has_value() ? std::to_string(*result) : std::string("NULL");

        std::string errorMessageStr =
            errorMessage.has_value() ? *errorMessage : std::string("NULL");

        std::stringstream sstream;
        sstream << "h1::entity::CalculationResultEntity{"
                << ".id=" << id << ";"
                << ".left=" << left << ";"
                << ".right=" << right << ";"
                << ".result=" << resultStr << ";"
                << ".operation=" << operation << ";"
                << ".statusCode=" << statusCode << ";"
                << ".errorMessage=" << errorMessageStr << ";"
                << "}";

        return sstream.str();
    }
};

} // namespace h1::entity

#endif
