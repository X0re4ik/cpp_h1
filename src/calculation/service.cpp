#include "h1/calculation/service.hpp"

namespace h1::calculation
{
CalculationService::CalculationService(db::DBManager& dbManager,
                                       lcache::CalcLocalCache& localCache) :
    dbManager_(dbManager),
    localCache_(localCache)
{}

ResultValue
    CalculationService::calculate(const jsonp::MathOperation& operation,
                                  calc::BaseSimpleCalculator& calculator)
{
    const auto key =
        lcache::OpTask{.left = static_cast<double>(operation.left),
                       .right = static_cast<double>(operation.right),
                       .operation = operation.operation};

    if (auto cached = localCache_.getIfExist(key))
    {
        return {cached->result, cached->errorMessage, cached->isSuccess, true};
    }

    bool succeeded = false;
    try
    {
        (void)calculator.calculate();
        succeeded = true;
    }
    catch (const calc::CalculatorException&)
    {}

    const auto& task = calculator.getTask();
    entity::CalculationResultEntity result;
    result.left = task.left;
    result.right = task.right;
    result.result =
        succeeded ? std::optional<double>{task.value} : std::nullopt;
    result.operation = operation.operation;
    result.statusCode = static_cast<int>(task.status);
    result.errorMessage = task.errorMessage;
    result.isSuccess = succeeded;

    if (succeeded)
    {
        dbManager_.registerOk(task.left, task.right, task.value,
                              operation.operation,
                              static_cast<int>(task.status));
    }
    else
    {
        dbManager_.registerError(task.left, task.right, operation.operation,
                                 static_cast<int>(task.status),
                                 task.errorMessage.value_or("UNKNOWN"));
    }

    localCache_.add(key, result);

    return {result.result, result.errorMessage, result.isSuccess, false};
}
} // namespace h1::calculation
