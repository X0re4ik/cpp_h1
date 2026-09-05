#ifndef H_H1_CALCULATOR_ENUMS_51342b8d7f3610d465e424f5193d521a
#define H_H1_CALCULATOR_ENUMS_51342b8d7f3610d465e424f5193d521a

#include <sys/types.h>
namespace h1::calc
{

enum class CalculatorTypeEnum : u_int8_t
{
    ADD = 0,
    SUBTRACT = 1,
    MULTIPLY = 2,
    DIVIDE = 3,
    FACTORIAL = 4,
    POWER = 5
};

} // namespace h1::calc

#endif