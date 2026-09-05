#ifndef H_H1_ARGPARSE_EXCEPTIONS_4b8a8ad4912ae99ce0a6ee2537c423d0
#define H_H1_ARGPARSE_EXCEPTIONS_4b8a8ad4912ae99ce0a6ee2537c423d0

#include "h1/common.hpp"

namespace h1::argp
{

class ArgParseException : public H1BaseException
{
  public:
    explicit ArgParseException(String msg) : H1BaseException(std::move(msg))
    {}
};

class NotFoundFileArgParseException : public ArgParseException
{
  public:
    explicit NotFoundFileArgParseException(const String& filePath) :
        ArgParseException("Не могу открыть файл по пути: " + filePath)
    {}
};

class InvalidJSONFormatArgParseException : public ArgParseException
{
  public:
    explicit InvalidJSONFormatArgParseException(const String& errMsg) :
        ArgParseException(
            "Входной параметр не соотвуетсвует формату JSON. Ошибка: " + errMsg)
    {}
};

} // namespace h1::argp

#endif