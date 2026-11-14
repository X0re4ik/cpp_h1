#pragma once
#ifndef H_H1_IO_SERVER_REQUEST_7c396a1e8b4d4f2ab8e5c0d9f631a274
#define H_H1_IO_SERVER_REQUEST_7c396a1e8b4d4f2ab8e5c0d9f631a274

#include "h1/common.hpp"
#include "h1/io_server/header.hpp"

#include <vector>

namespace h1::io_server
{

// Структура HTTP-запроса
struct Request
{
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    String method;

    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    String uri;

    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    int httpVersionMajor;

    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    int httpVersionMinor;

    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    std::vector<Header> headers;

    /// Тело запроса (payload). Заполняется парсером при наличии
    /// заголовка Content-Length.
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    String body;
};

} // namespace h1::io_server

#endif
