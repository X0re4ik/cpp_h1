#ifndef H_H1_LOGGER_EXCEPTIONS_bcefa705229ee883c5c6b6c46a08323d
#define H_H1_LOGGER_EXCEPTIONS_bcefa705229ee883c5c6b6c46a08323d

#include "h1/common.hpp"
#include "h1/logger/code_location.hpp"

namespace h1::log
{

class LoggerConfigException : public H1BaseException
{
  public:
    explicit LoggerConfigException(String msg) : H1BaseException(std::move(msg))
    {}
};

class LogFileNotFound : public LoggerConfigException
{
  public:
    explicit LogFileNotFound() :
        LoggerConfigException(
            R"(Файл не установлен, 
перед тем, как вызывать метод `getFileLog`, 
необходимо установить addFileLog("/tmp/example.log"))")
    {}
};

} // namespace h1::log

#endif