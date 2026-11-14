#pragma once
#ifndef H_H1_IO_SERVER_REQUEST_PARSER_9f2d5e7c1a3b4c6d8e0f2a5b7c9d1e3f
#define H_H1_IO_SERVER_REQUEST_PARSER_9f2d5e7c1a3b4c6d8e0f2a5b7c9d1e3f

#include "h1/io_server/request.hpp"

#include <boost/logic/tribool.hpp>
#include <boost/tuple/tuple.hpp>

namespace h1::io_server
{

/// Парсер входящих HTTP-запросов.
class RequestParser
{
  public:
    RequestParser();

    // Сброс парсера в начальное состояние
    void reset();

    /// Разобрать часть данных. Возвращаемый tribool равен true, когда
    /// запрос разобран полностью, false — если данные некорректны,
    /// indeterminate — когда нужны ещё данные. Итератор показывает,
    /// сколько входных данных было использовано.
    template <typename InputIterator>
    boost::tuple<boost::tribool, InputIterator>
        parse(Request& req, InputIterator begin, InputIterator end)
    {
        while (begin != end)
        {
            boost::tribool result = consume(req, *begin++);
            if (result || !result)
            {
                return boost::make_tuple(result, begin);
            }
        }
        boost::tribool result = boost::indeterminate;
        return boost::make_tuple(result, begin);
    }

  private:
    /// Обработать очередной символ входного потока
    boost::tribool consume(Request& req, char input);

    /// Является ли байт символом HTTP
    static bool isChar(int c);

    /// Является ли байт управляющим символом HTTP
    static bool isCtl(int c);

    /// Является ли байт специальным символом HTTP (tspecial)
    static bool isTspecial(int c);

    /// Является ли байт цифрой
    static bool isDigit(int c);

    /// Текущее состояние парсера
    enum class State
    {
        MethodStart,
        Method,
        Uri,
        HttpVersionH,
        HttpVersionT1,
        HttpVersionT2,
        HttpVersionP,
        HttpVersionSlash,
        HttpVersionMajorStart,
        HttpVersionMajor,
        HttpVersionMinorStart,
        HttpVersionMinor,
        ExpectingNewline1,
        HeaderLineStart,
        HeaderLws,
        HeaderName,
        SpaceBeforeHeaderValue,
        HeaderValue,
        ExpectingNewline2,
        ExpectingNewline3,
        Body
    } state_;

    /// Сколько байт тела осталось прочитать
    std::size_t bodyRemaining_;

    /// Найти Content-Length в заголовках (регистр имени не учитывается).
    /// Возвращает false, если заголовок задан некорректно.
    static bool extractContentLength(const Request& req, std::size_t& length);
};

} // namespace h1::io_server

#endif
