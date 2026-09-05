#include "h1/json_parser/impl.hpp"

#include "h1/json_parser/exceptions.hpp"

#include <string>
#include <unordered_map>

namespace h1::jsonp
{

// NOLINTNEXTLINE(readability-identifier-naming,misc-use-internal-linkage)
void from_json(const nlohmann::json& jsonData, MathOperation& mathOp)
{

    const String operationName = "operation";

    const std::set<std::string> allowedKeys = {"operation", "left", "right",
                                               "value"};

    for (const auto& [key, value] : jsonData.items())
    {
        (void)value;
        if (!allowedKeys.contains(key))
        {
            throw std::invalid_argument("Неизветный ключ '" + key + "'");
        }
    }

    jsonData.at(operationName).get_to(mathOp.operation);

    if (mathOp.operation == "!")
    {
        if (!jsonData.at("value").is_number_integer())
        {
            throw std::invalid_argument("`value` должно быть типа integer");
        }
        mathOp.right = 0;
        jsonData.at("value").get_to(mathOp.left);
        return;
    }

    jsonData.at("left").get_to(mathOp.left);
    jsonData.at("right").get_to(mathOp.right);

    if (!jsonData["left"].is_number_integer())
    {
        throw std::invalid_argument("`left` должно быть типа integer");
    }

    if (!jsonData["right"].is_number_integer())
    {
        throw std::invalid_argument("`right` должно быть типа integer");
    }

    const std::set<std::string> validOps = {"+", "-", "*", "/", "^"};
    if (!validOps.contains(mathOp.operation))
    {
        throw std::invalid_argument(
            "Неизвестная операция: " + mathOp.operation +
            ". Разрешены: +, -, *, /, !, ^");
    }
}

MathOperation CalculatorParseJson::parse(const String& jsonString)
{
    using json = nlohmann::json;

    try
    {
        auto data = json::parse(jsonString);
        auto binaryMathOperation = data.get<MathOperation>();
        return data;
    }
    catch (const json::exception& exc)
    {
        throw JsonParseException(exc.what());
    }
    catch (const std::invalid_argument& exc)
    {
        throw JsonParseException(exc.what());
    }
    catch (...)
    {
        throw JsonParseException("Неизвестная ошибка парсинга сообщения");
    }
}

} // namespace h1::jsonp
