#pragma once
#ifndef H_H1_IO_SERVER_HEADER_3a1b9d0c5fe74a2fb6c8d4e90a52b731
#define H_H1_IO_SERVER_HEADER_3a1b9d0c5fe74a2fb6c8d4e90a52b731

#include "h1/common.hpp"

namespace h1::io_server
{

// Структура заголовка HTTP-запроса
struct Header
{
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    String name;

    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    String value;
};

} // namespace h1::io_server

#endif
