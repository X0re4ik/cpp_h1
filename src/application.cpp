#include "h1/application.hpp"

#include "h1/common.hpp"
#include "h1/db/db.hpp"
#include "h1/db/manager.hpp"
#include "h1/exceptions.hpp"
#include "h1/json_parser/impl.hpp"
#include "h1/local_cache/local_cache.hpp"
#include "h1/logger/logger.hpp"

#include <exception>
#include <iomanip>
#include <iostream>
#include <optional>
#include <string>

namespace h1
{
namespace
{

void initLogger(bool verbose)
{

    using h1::log::initLoggerConfig;
    using h1::log::LoggerConfig;
    using h1::log::LoggerLevelEnum;
    const auto logLevel =
        verbose ? LoggerLevelEnum::DEBUG : LoggerLevelEnum::OFF;
    initLoggerConfig(LoggerConfig()
                         .addConsoleLog()
                         .setPattern("[%H:%M:%S.%f] [%^%l%$] [%t] [%s:%#] %v")
                         .setLogLevel(logLevel)
                         .setLogName(h1::projectName));
}

void warmUpCache(h1::lcache::CalcLocalCache& localCache,
                 h1::db::DBManager& dbManager)
{
    const int limitDBData = 100;
    auto tasks = dbManager.getData(limitDBData);

    for (const auto& task : tasks)
    {
        auto opTask = h1::lcache::OpTask{
            .left = task.left,
            .right = task.right,
            .operation = task.operation,
        };
        localCache.add(opTask, task);
    }
}

double calcResult(const h1::jsonp::MathOperation& oper,
                  h1::calculation::CalculationService& service,
                  h1::calc::BaseSimpleCalculator& simpleCalc)
{
    auto result = service.calculate(oper, simpleCalc);

    String message = result.fromCache ? "КЭШ-а hit" : "КЭШ-а miss";

    H1_LOG_INFO(message);

    if (!result.isSuccess)
    {
        throw h1::calc::CalculatorException(
            result.errorMessage.value_or("UNKNOWN"));
    }

    return result.result.value();
}

h1::calc::CalculatorTypeEnum char2CalculatorType(const String& operationChar)
{
    if (operationChar.length() != 1)
    {
        throw AppException("Неизвестный тип операции: " + operationChar);
    }

    switch (operationChar[0])
    {
        case '+':
            return h1::calc::CalculatorTypeEnum::ADD;
        case '-':
            return h1::calc::CalculatorTypeEnum::SUBTRACT;
        case '*':
            return h1::calc::CalculatorTypeEnum::MULTIPLY;
        case '/':
            return h1::calc::CalculatorTypeEnum::DIVIDE;
        case '!':
            return h1::calc::CalculatorTypeEnum::FACTORIAL;
        case '^':
            return h1::calc::CalculatorTypeEnum::POWER;
        default:
            throw AppException("Неизвестный тип операции: " + operationChar);
    }
}

String prettyPrintMathOp(const h1::jsonp::MathOperation& mathOper)
{
    if (mathOper.operation == "!")
    {
        return std::to_string(mathOper.left) + mathOper.operation;
    }

    return std::to_string(mathOper.left) + mathOper.operation +
           std::to_string(mathOper.right);
}

void prettyPrintResult(std::ostream& oStream, double result,
                       const h1::jsonp::MathOperation& mathOperation)
{
    oStream << prettyPrintMathOp(mathOperation) << "=" << std::fixed
            << std::setprecision(2) << result << '\n';
}

} // namespace
Application::Application(h1::db::DBManager& dbManager,
                         h1::lcache::CalcLocalCache& localCache,
                         std::ostream& oStream, std::ostream& eStream) :
    oStream_(oStream),
    eStream_(eStream), argParse_(projectName, projectVersion),
    dbManager_(dbManager), localCache_(localCache)
{}
int Application::run(int argc, char** argv)
{
    try
    {

        auto value = argParse_.parse(argc, argv);
        initLogger(value.verbose);

        H1_LOG_INFO("Аргументы командной строки обработаны");

        warmUpCache(localCache_, dbManager_);
        H1_LOG_INFO("Кэш успешно прогрет");

        auto mathOperation =
            h1::jsonp::CalculatorParseJson::parse(value.jsonValue);

        H1_LOG_INFO("Определена математическая операция (" +
                    mathOperation.operation + ")");
        auto calcType = char2CalculatorType(mathOperation.operation);

        auto calculator = h1::calc::CalculatorFactory::makeCalculator(
            calcType, mathOperation.left, mathOperation.right);
        h1::calculation::CalculationService calculationService(dbManager_,
                                                               localCache_);
        auto result =
            calcResult(mathOperation, calculationService, *calculator);

        H1_LOG_INFO("Результат успешно рассчитан (" + std::to_string(result) +
                    ")")
        prettyPrintResult(oStream_.get(), result, mathOperation);
        return 0;
    }
    catch (const H1BaseException& exc)
    {
        const auto& error = exc.what();
        H1_LOG_ERROR(error)
        eStream_.get() << error << '\n';
        return 1;
    }
    catch (const std::exception& exc)
    {
        const auto& error = exc.what();
        H1_LOG_ERROR(error)
        eStream_.get() << error << '\n';
        return 1;
    }
    catch (...)
    {
        eStream_.get() << "Критическая ошибка работы программы" << '\n';
        return 1;
    }
    return 0;
}

} // namespace h1
