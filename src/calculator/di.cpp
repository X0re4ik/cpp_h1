#include "h1/calculator/di.hpp"

namespace h1::calc
{

CalculatorFactory::CalculatorPtr
    CalculatorFactory::makeCalculator(const CalculatorTypeEnum& calcEnum,
                                      ISimpleCalculator::InputValue leftValue,
                                      ISimpleCalculator::InputValue rightValue)
{

    auto task = BaseSimpleCalculator::Task{.left = leftValue,
                                           .right = rightValue,
                                           .status = OperationStatus::NOT_STATE,
                                           .value = 0};
    switch (calcEnum)
    {
        case CalculatorTypeEnum::ADD:
            return std::make_unique<AdditionCalculator>(task);

        case CalculatorTypeEnum::SUBTRACT:
            return std::make_unique<SubtractionCalculator>(task);

        case CalculatorTypeEnum::MULTIPLY:
            return std::make_unique<MultiplicationCalculator>(task);

        case CalculatorTypeEnum::DIVIDE:
            return std::make_unique<DivisionCalculator>(task);

        case CalculatorTypeEnum::FACTORIAL:
            return std::make_unique<FactorialCalculator>(task);

        case CalculatorTypeEnum::POWER:
            return std::make_unique<PowerCalculator>(task);

        default:
            throw UnknownCalculationTypeException(calcEnum);
    }
}
} // namespace h1::calc
