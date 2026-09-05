#ifndef H_H1_CALCULATOR_DI_4c1e0ff655ec562f6b9f2c75d238467b
#define H_H1_CALCULATOR_DI_4c1e0ff655ec562f6b9f2c75d238467b

#include "./calc.hpp"
#include "./enums.hpp"
#include "./exceptions.hpp"
#include "h1.hpp"
#include "h1/common.hpp"

#include <memory>

namespace h1::calc
{
class CalculatorFactory
{
  public:
    using CalculatorPtr = std::unique_ptr<ISimpleCalculator>;

    static CalculatorPtr
        makeCalculator(const CalculatorTypeEnum& calcEnum,
                       ISimpleCalculator::InputValue leftValue,
                       ISimpleCalculator::InputValue rightValue) H1_EXCEPT;
};
} // namespace h1::calc

#endif