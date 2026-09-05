#ifndef H_H1_JSON_PARSER_EXCEPTIONS_39108350d2b2f65aff2d12acc9817e4f
#define H_H1_JSON_PARSER_EXCEPTIONS_39108350d2b2f65aff2d12acc9817e4f

#include "h1/common.hpp"

#include <string>

namespace h1::jsonp
{

class JsonParseException : public H1BaseException
{
  public:
    explicit JsonParseException(String msg) : H1BaseException(std::move(msg))
    {}
};

} // namespace h1::jsonp

#endif