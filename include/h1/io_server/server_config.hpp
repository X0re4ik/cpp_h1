#pragma once
#ifndef H_H1_IO_SERVER_SERVER_CONFIG_5f6d8a1c2b3e4f7a9c0d1e2f3a4b5c6d
#define H_H1_IO_SERVER_SERVER_CONFIG_5f6d8a1c2b3e4f7a9c0d1e2f3a4b5c6d

#include "h1/common.hpp"

#include <cstddef>
#include <cstdint>

namespace h1::io_server
{

/// Конфигурация серверного режима.
struct ServerConfig
{
    String host{"0.0.0.0"};
    std::uint16_t port{8080};
    std::size_t threads{4};
    bool verbose{false};

    /// Разобрать параметры CLI и переменные окружения.
    /// Приоритет: CLI -> H1_SERVER_* -> значения по умолчанию.
    /// NOLINTNEXTLINE(modernize-avoid-c-arrays,cppcoreguidelines-avoid-c-arrays)
    static ServerConfig parse(int argc, char* argv[]);
};

} // namespace h1::io_server

#endif
