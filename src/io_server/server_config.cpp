#include "h1/io_server/server_config.hpp"

#include "h1/common.hpp"
#include "h1/exceptions.hpp"

#include <argparse/argparse.hpp>
#include <libenvpp/env.hpp>

#include <cstddef>
#include <cstdint>
#include <exception>
#include <stdexcept>
#include <string>

namespace h1::io_server
{

namespace
{

constexpr auto defaultHost = "0.0.0.0";
constexpr std::uint16_t defaultPort = 8080;
constexpr std::size_t defaultThreads = 4;
constexpr std::size_t minThreads = 2;
constexpr std::size_t maxThreads = 256;
constexpr std::uint32_t maxPort = 65535;

std::uint16_t parsePort(const String& value)
{
    try
    {
        const auto parsed = std::stoul(value);
        if (parsed == 0 || parsed > maxPort)
        {
            throw std::out_of_range("port");
        }
        return static_cast<std::uint16_t>(parsed);
    }
    catch (const std::exception&)
    {
        throw AppException("Некорректный порт сервера: '" + value + "'");
    }
}

std::size_t parseThreads(const String& value)
{
    try
    {
        const auto parsed = std::stoul(value);
        if (parsed < minThreads || parsed > maxThreads)
        {
            throw std::out_of_range("threads");
        }
        return static_cast<std::size_t>(parsed);
    }
    catch (const std::exception&)
    {
        throw AppException("Некорректное число потоков сервера: '" + value +
                           "' (допустимо " + std::to_string(minThreads) + ".." +
                           std::to_string(maxThreads) + ")");
    }
}

} // namespace

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays)
ServerConfig ServerConfig::parse(int argc, char* argv[])
{
    argparse::ArgumentParser program(String{h1::projectName} + "Server",
                                     h1::projectVersion);

    program.add_argument("--host").help(
        "Адрес, на котором слушает сервер (env: H1_SERVER_HOST; "
        "по умолчанию 0.0.0.0)");

    program.add_argument("--port").help(
        "TCP-порт сервера (env: H1_SERVER_PORT; по умолчанию 8080)");

    program.add_argument("--threads")
        .help("Количество потоков обработки, минимум 2 "
              "(env: H1_SERVER_THREADS; по умолчанию 4)");

    program.add_argument("--verbose")
        .help("Подробный лог работы сервера")
        .default_value(false)
        .implicit_value(true);

    try
    {
        program.parse_args(argc, argv);
    }
    catch (const std::exception& exc)
    {
        throw AppException("Ошибка разбора аргументов сервера: " +
                           String{exc.what()});
    }

    ServerConfig config;
    config.verbose = program.get<bool>("--verbose");

    const auto hostArg = program.present<String>("--host");
    config.host =
        hostArg.value_or(env::get_or<String>("H1_SERVER_HOST", defaultHost));

    const auto portArg = program.present<String>("--port");
    config.port =
        portArg.has_value()
            ? parsePort(*portArg)
            : env::get_or<std::uint16_t>("H1_SERVER_PORT", defaultPort);

    const auto threadsArg = program.present<String>("--threads");
    config.threads =
        threadsArg.has_value()
            ? parseThreads(*threadsArg)
            : env::get_or<std::size_t>("H1_SERVER_THREADS", defaultThreads);

    if (config.threads < minThreads)
    {
        throw AppException("Количество потоков сервера должно быть не меньше " +
                           std::to_string(minThreads));
    }

    return config;
}

} // namespace h1::io_server
