#include "h1/io_server/handlers/task_manager.hpp"

#include "h1/application.hpp"
#include "h1/common.hpp"
#include "h1/db/db.hpp"
#include "h1/db/manager.hpp"
#include "h1/exceptions.hpp"
#include "h1/json_parser/impl.hpp"
#include "h1/json_parser/json_parser.hpp"
#include "h1/local_cache/local_cache.hpp"
#include "h1/logger/logger.hpp"

#include <exception>
#include <iomanip>
#include <iostream>
#include <memory>
#include <optional>
#include <string>

namespace h1::io_server
{

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

double calcResult(const h1::jsonp::MathOperation& oper,
                  h1::calculation::CalculationService& service,
                  h1::calc::BaseSimpleCalculator& simpleCalc)
{
    auto result = service.calculate(oper, simpleCalc);

    const String message = result.fromCache ? "КЭШ-а hit" : "КЭШ-а miss";

    H1_LOG_INFO(message);

    if (!result.isSuccess)
    {
        throw h1::calc::CalculatorException(
            result.errorMessage.value_or("UNKNOWN"));
    }

    return result.result.value();
}

class SmartCalculatorService : public ISmartCalculatorService
{
  public:
    SmartCalculatorService(h1::db::IDBManager& dbManager,
                           h1::lcache::ICalcLocalCache& localCache) :
        dbManager_(dbManager),
        localCache_(localCache)
    {}

    void calculate(const String& jsonData) override
    {
        auto mathOperation = h1::jsonp::CalculatorParseJson::parse(jsonData);

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
                    ")");
    }

  private:
    h1::db::IDBManager& dbManager_;
    h1::lcache::ICalcLocalCache& localCache_;
};

SmartCalculatorServicePtr
    makeSmartCalculatorService(h1::db::IDBManager& dbManager,
                               h1::lcache::ICalcLocalCache& localCache)
{
    return std::make_unique<SmartCalculatorService>(dbManager, localCache);
}

HandlerPtr makeTaskManagerHandler(h1::db::IDBManager& dbManager,
                                  h1::lcache::ICalcLocalCache& localCache)
{

    auto calculatorService = makeSmartCalculatorService(dbManager, localCache);
    return std::make_shared<TaskManagerHandler>(std::move(calculatorService));
}

TaskManagerHandler::TaskManagerHandler(
    SmartCalculatorServicePtr smartCalculatorServicePtr) :
    smartCalculatorServicePtr_(std::move(smartCalculatorServicePtr))
{}

void TaskManagerHandler::handleRequest(const Request& request,
                                       Response& response) const
{
    response.status = Response::Status::Ok;
    response.headers = {
        Header{"Content-Length", std::to_string(request.body.size())},
    };

    smartCalculatorServicePtr_->calculate(request.body);

    response.content = request.body;
}

} // namespace h1::io_server