#pragma once

#include "h1/calculator/calc.hpp"
#include "h1/db/manager.hpp"
#include "h1/json_parser/impl.hpp"
#include "h1/local_cache/local_cache.hpp"

#include <optional>

namespace h1::calculation
{
struct ResultValue
{
    std::optional<double> result;
    std::optional<String> errorMessage;
    bool isSuccess = false;
    bool fromCache = false;
};

class CalculationService
{
  public:
    CalculationService(db::DBManager& dbManager,
                       lcache::CalcLocalCache& localCache);
    ResultValue calculate(const jsonp::MathOperation& operation,
                          calc::BaseSimpleCalculator& calculator);

  private:
    db::DBManager& dbManager_;
    lcache::CalcLocalCache& localCache_;
};
} // namespace h1::calculation
