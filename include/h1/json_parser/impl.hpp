#ifndef H_H1_JSON_PARSER_IMPL_ab1a45f69d680ff1e886bc2a8ebe4238
#define H_H1_JSON_PARSER_IMPL_ab1a45f69d680ff1e886bc2a8ebe4238

#include "./exceptions.hpp"
#include "h1/common.hpp"

#include <nlohmann/json.hpp>

#include <iostream>
#include <set>

namespace h1::jsonp
{

struct MathOperation
{
    using Value = int;
    Value left;
    Value right;
    String operation;
};

class CalculatorParseJson
{

  public:
    H1_NODISCARD static MathOperation parse(const String& jsonString) H1_EXCEPT;
};
} // namespace h1::jsonp

#endif