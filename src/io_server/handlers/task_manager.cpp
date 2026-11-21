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

#include <nlohmann/json.hpp>

#include <exception>
#include <memory>
#include <optional>
#include <string>
#include <utility>

namespace h1::io_server
{

namespace
{

using json = nlohmann::json;

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

void setJsonResponse(Response& response, Response::Status status,
                     const json& body)
{
    response.status = status;
    response.content = body.dump();
    response.headers = {
        Header{.name = "Content-Length",
               .value = std::to_string(response.content.size())},
        Header{.name = "Content-Type", .value = "application/json"},
    };
}

void setErrorResponse(Response& response, const String& message)
{
    setJsonResponse(response, Response::Status::InternalServerError,
                    json{{"error", message}});
}

} // namespace

class SmartCalculatorService : public ISmartCalculatorService
{
  public:
    SmartCalculatorService(h1::db::IDBManager& dbManager,
                           h1::lcache::ICalcLocalCache& localCache) :
        dbManager_(dbManager), localCache_(localCache)
    {}

    CalculationReply calculate(const String& jsonData) override
    {
        const auto mathOperation =
            h1::jsonp::CalculatorParseJson::parse(jsonData);

        H1_LOG_INFO("Определена математическая операция (" +
                    mathOperation.operation + ")");
        const auto calcType = char2CalculatorType(mathOperation.operation);

        auto calculator = h1::calc::CalculatorFactory::makeCalculator(
            calcType, mathOperation.left, mathOperation.right);
        h1::calculation::CalculationService calculationService(dbManager_,
                                                               localCache_);
        const auto result =
            calculationService.calculate(mathOperation, *calculator);

        CalculationReply reply;
        reply.operation = mathOperation.operation;
        reply.left = static_cast<double>(mathOperation.left);
        reply.right = static_cast<double>(mathOperation.right);
        reply.result = result.result;
        reply.errorMessage = result.errorMessage;
        reply.isSuccess = result.isSuccess;
        reply.fromCache = result.fromCache;

        if (reply.isSuccess)
        {
            H1_LOG_INFO("Результат успешно рассчитан (" +
                        std::to_string(reply.result.value_or(0.0)) + ")");
        }
        else
        {
            H1_LOG_ERROR("Ошибка расчёта: " +
                         reply.errorMessage.value_or("UNKNOWN"));
        }

        return reply;
    }

  private:
    h1::db::IDBManager& dbManager_;
    h1::lcache::ICalcLocalCache& localCache_;
};

namespace
{

SmartCalculatorServicePtr
    makeSmartCalculatorService(h1::db::IDBManager& dbManager,
                               h1::lcache::ICalcLocalCache& localCache)
{
    return std::make_unique<SmartCalculatorService>(dbManager, localCache);
}

} // namespace

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
    try
    {
        const auto reply = smartCalculatorServicePtr_->calculate(request.body);

        if (!reply.isSuccess)
        {
            setErrorResponse(response, reply.errorMessage.value_or("UNKNOWN"));
            return;
        }

        json body;
        body["operation"] = reply.operation;
        if (reply.operation == "!")
        {
            body["value"] = reply.left;
        }
        else
        {
            body["left"] = reply.left;
            body["right"] = reply.right;
        }
        body["result"] = reply.result.value_or(0.0);
        body["fromCache"] = reply.fromCache;

        setJsonResponse(response, Response::Status::Ok, body);
    }
    catch (const std::exception& exc)
    {
        setErrorResponse(response, exc.what());
    }
    catch (...)
    {
        setErrorResponse(response, "Неизвестная ошибка сервера");
    }
}

} // namespace h1::io_server
