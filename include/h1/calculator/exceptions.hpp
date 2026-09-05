#ifndef H_H1_CALCULATOR_EXCEPTIONS_32bd6cd37d6bffa49a64077f7f524af4
#define H_H1_CALCULATOR_EXCEPTIONS_32bd6cd37d6bffa49a64077f7f524af4

#include "./enums.hpp"
#include "h1/common.hpp"

#include <string>

namespace h1::calc
{

class CalculatorException : public H1BaseException
{
  public:
    explicit CalculatorException(String msg) : H1BaseException(std::move(msg))
    {}
};

class UnknownCalculationTypeException : public CalculatorException
{
  public:
    explicit UnknownCalculationTypeException(
        const CalculatorTypeEnum& calcEnum) :
        CalculatorException("Неизвестный тип операции: " +
                            std::to_string(static_cast<int>(calcEnum)))
    {}
};

} // namespace h1::calc

#endif