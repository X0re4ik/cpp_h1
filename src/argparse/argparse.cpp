#include "h1/argparse/argparse.hpp"

#include "h1/argparse/exceptions.hpp"
#include "h1/common.hpp"

#include <argparse/argparse.hpp>
#include <nlohmann/json.hpp>

#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace h1::argp
{

using json = nlohmann::json;

H1ArgParse::H1ArgParse(String programName) :
    programName_(std::move(programName)), version_("0.1")
{}

H1ArgParse::H1ArgParse(String programName, String version) :
    programName_(std::move(programName)), version_(std::move(version))
{}

// NOLINTNEXTLINE(modernize-avoid-c-arrays,cppcoreguidelines-avoid-c-arrays)
ArgsValue H1ArgParse::parse(int argc, char* argv[])
{
    argparse::ArgumentParser program(programName_, version_);

    program.add_argument("--config")
        .help(R"(JSON строка.
Пример JSON:
1) +:
    '{"left": 2, "right": 2, "operation": "+"}'
2) -:
    '{"left": 2, "right": 2, "operation": "-"}'
3) *:
    '{"left": 2, "right": 2, "operation": "*"}'
4) /:
    '{"left": 2, "right": 2, "operation": "/"}'
5) ^:
    '{"left": 2, "right": 2, "operation": "^"}'
6) !:
    '{"value": 2, "operation": "!"}'

Example:
    h1 --config '{"left": 2, "right": 2, "operation": "^"}'
    h1 --config '{"value": 2, "operation": "!"}'
)")
        .required();

    program.add_argument("--verbose")
        .help(R"(Подробный лог работы программы)")
        .default_value(false)
        .implicit_value(true);

    try
    {
        program.parse_args(argc, argv);

        auto inputJson = program.get<String>("--config");
        auto verbose = program.get<bool>("--verbose");

        return {.jsonValue = inputJson, .verbose = verbose};
    }
    catch (const std::exception& exc)
    {
        const String strMsg = exc.what();
        throw ArgParseException("Ошибка парсинга сообщения: \n\t" + strMsg);
    }
    catch (...)
    {
        throw ArgParseException("Неизвестная ошибка при обработке сообщения");
    }
}

} // namespace h1::argp