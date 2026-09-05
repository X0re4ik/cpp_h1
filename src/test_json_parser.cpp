#include "h1/json_parser/exceptions.hpp"
#include "h1/json_parser/impl.hpp"
#include "h1/json_parser/json_parser.hpp"

#include <nlohmann/json.hpp>

#include <array>
#include <string>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

namespace
{

struct ValidJSONTestCase
{
    std::string validJSONInput;
    int left;
    int right;
    std::string operation;
};

class ValidJSONTest : public testing::TestWithParam<ValidJSONTestCase>
{};

TEST_P(ValidJSONTest, TestCaseParseValidJSON)
{
    // Arrage
    const auto& testCase = GetParam();

    // Act
    auto data = h1::jsonp::CalculatorParseJson::parse(testCase.validJSONInput);

    // Assert
    ASSERT_EQ(data.operation, testCase.operation);
    ASSERT_EQ(data.left, testCase.left);
    ASSERT_EQ(data.right, testCase.right);
}

INSTANTIATE_TEST_SUITE_P(
    TestCaseParseValidJSON, ValidJSONTest,
    testing::Values(
        // Базовые операции
        ValidJSONTestCase{R"({"left": 5, "right": 3, "operation": "+"})", 5, 3,
                          "+"},
        ValidJSONTestCase{R"({"left": 10, "right": 4, "operation": "-"})", 10,
                          4, "-"},
        ValidJSONTestCase{R"({"left": 7, "right": 8, "operation": "*"})", 7, 8,
                          "*"},
        ValidJSONTestCase{R"({"left": 15, "right": 3, "operation": "/"})", 15,
                          3, "/"},

        // Отрицательные числа
        ValidJSONTestCase{R"({"left": -5, "right": 3, "operation": "+"})", -5,
                          3, "+"},
        ValidJSONTestCase{R"({"left": 10, "right": -4, "operation": "-"})", 10,
                          -4, "-"},
        ValidJSONTestCase{R"({"left": -7, "right": -8, "operation": "*"})", -7,
                          -8, "*"},
        ValidJSONTestCase{R"({"left": -15, "right": -3, "operation": "/"})",
                          -15, -3, "/"},

        // Ноль в операндах
        ValidJSONTestCase{R"({"left": 0, "right": 5, "operation": "+"})", 0, 5,
                          "+"},
        ValidJSONTestCase{R"({"left": 10, "right": 0, "operation": "-"})", 10,
                          0, "-"},
        ValidJSONTestCase{R"({"left": 0, "right": 7, "operation": "*"})", 0, 7,
                          "*"},
        ValidJSONTestCase{R"({"left": 0, "right": 5, "operation": "/"})", 0, 5,
                          "/"},

        // Большие числа
        ValidJSONTestCase{
            R"({"left": 1000000, "right": 500000, "operation": "+"})", 1000000,
            500000, "+"},
        ValidJSONTestCase{R"({"left": 999999, "right": 1, "operation": "-"})",
                          999999, 1, "-"},
        ValidJSONTestCase{R"({"left": 12345, "right": 6789, "operation": "*"})",
                          12345, 6789, "*"},
        ValidJSONTestCase{R"({"left": 1000000, "right": 2, "operation": "/"})",
                          1000000, 2, "/"},

        // Различные операции с отрицательными результатами
        ValidJSONTestCase{R"({"left": 3, "right": 10, "operation": "-"})", 3,
                          10, "-"},
        ValidJSONTestCase{R"({"left": -3, "right": 10, "operation": "+"})", -3,
                          10, "+"},
        ValidJSONTestCase{R"({"left": 5, "right": -10, "operation": "*"})", 5,
                          -10, "*"},
        ValidJSONTestCase{R"({"left": 10, "right": -2, "operation": "/"})", 10,
                          -2, "/"},

        // Деление с остатком
        ValidJSONTestCase{R"({"left": 17, "right": 5, "operation": "/"})", 17,
                          5, "/"},
        ValidJSONTestCase{R"({"left": 22, "right": 7, "operation": "/"})", 22,
                          7, "/"},

        // Простые случаи для каждой операции
        ValidJSONTestCase{R"({"left": 1, "right": 1, "operation": "+"})", 1, 1,
                          "+"},
        ValidJSONTestCase{R"({"left": 1, "right": 1, "operation": "-"})", 1, 1,
                          "-"},
        ValidJSONTestCase{R"({"left": 2, "right": 3, "operation": "*"})", 2, 3,
                          "*"},
        ValidJSONTestCase{R"({"left": 6, "right": 2, "operation": "/"})", 6, 2,
                          "/"},
        // Факториал
        ValidJSONTestCase{R"({"value": 10000, "operation": "!"})", 10000, 0,
                          "!"},
        // Возведение в степень
        ValidJSONTestCase{
            R"({"left": 10000, "right": 10000, "operation": "^"})", 10000,
            10000, "^"}));

struct InvalidJSONTestCase
{
    std::string invalidJSONInput;
};

class InvalidJSONTest : public testing::TestWithParam<InvalidJSONTestCase>
{};

TEST_P(InvalidJSONTest, TestCaseParseInvalidJSON)
{
    // Arrage
    using h1::jsonp::CalculatorParseJson;
    using h1::jsonp::JsonParseException;
    const auto& testCase = GetParam();

    // Act + Assert
    ASSERT_THROW((void)CalculatorParseJson::parse(testCase.invalidJSONInput),
                 JsonParseException);
}

INSTANTIATE_TEST_SUITE_P(
    TestCaseParseInvalidJSON, InvalidJSONTest,
    testing::Values(
        // Неправильные имена полей
        InvalidJSONTestCase{
            R"({"left": 10000, "right": 10000, "operation1": "^"})"},
        InvalidJSONTestCase{R"({"leftt": 5, "right": 3, "operation": "+"})"},
        InvalidJSONTestCase{R"({"left": 5, "rightt": 3, "operation": "+"})"},
        InvalidJSONTestCase{R"({"left": 5, "right": 3, "operatio": "+"})"},

        // Отсутствующие поля
        InvalidJSONTestCase{
            R"({"left": 5, "right": 3})" // нет operation
        },
        InvalidJSONTestCase{
            R"({"left": 5, "operation": "+"})" // нет right
        },
        InvalidJSONTestCase{
            R"({"right": 3, "operation": "+"})" // нет left
        },
        InvalidJSONTestCase{
            R"({"left": 5, "right": 3, "operation": ""})" // пустая операция
        },

        // Неверные типы данных
        InvalidJSONTestCase{
            R"({"left": "5", "right": 3, "operation": "+"})" // left как строка
        },
        InvalidJSONTestCase{
            R"({"left": 5, "right": "3", "operation": "+"})" // right как строка
        },
        InvalidJSONTestCase{
            R"({"left": 5, "right": 3, "operation": 123})" // operation как число
        },
        InvalidJSONTestCase{
            R"({"left": 5.5, "right": 3, "operation": "+"})" // left как float
        },
        InvalidJSONTestCase{
            R"({"left": 5, "right": 3.14, "operation": "+"})" // right как float
        },
        InvalidJSONTestCase{
            R"({"value": 5.5, "operation": "!"})" // value как float
        },
        InvalidJSONTestCase{R"("left" : 1, "right" : 1, "operation" : "!"})"},
        // Некорректный JSON синтаксис
        InvalidJSONTestCase{
            R"({"left": 5, "right": 3, "operation": "+")" // отсутствует закрывающая скобка
        },
        InvalidJSONTestCase{
            R"({"left": 5, "right": 3, "operation": "+",})" // лишняя запятая
        },
        InvalidJSONTestCase{
            R"({left: 5, right: 3, operation: "+"})" // нет кавычек у ключей
        },
        InvalidJSONTestCase{
            R"({"left": 5, "right": 3, "operation": '+'})" // одинарные кавычки вместо двойных
        },
        InvalidJSONTestCase{
            R"({"left": 5, "right": , "operation": "+"})" // пустое значение right
        },
        InvalidJSONTestCase{
            R"({"left": 5, "right": 3, "operation": null})" // operation = null
        },
        InvalidJSONTestCase{
            R"({"left": null, "right": 3, "operation": "+"})" // left = null
        },
        InvalidJSONTestCase{
            R"({"left": 5, "right": null, "operation": "+"})" // right = null
        },

        // Недопустимые операции
        InvalidJSONTestCase{
            R"({"left": 5, "right": 3, "operation": "%"})" // остаток от деления
        },
        InvalidJSONTestCase{
            R"({"left": 5, "right": 3, "operation": "=="})" // сравнение
        },
        InvalidJSONTestCase{
            R"({"left": 5, "right": 3, "operation": "**"})" // неподдерживаемая операция
        },
        InvalidJSONTestCase{
            R"({"left": 5, "right": 3, "operation": "plus"})" // операция словом
        },

        // Специальные символы в JSON
        InvalidJSONTestCase{
            R"({"left": 5, "right": 3, "operation": "+", "extra": "data"})" // лишнее поле
        },

        // Пробелы и пустые строки
        InvalidJSONTestCase{
            "" // пустая строка
        },
        InvalidJSONTestCase{
            "   " // только пробелы
        },
        InvalidJSONTestCase{
            "{}" // пустой объект
        }));

} // namespace
