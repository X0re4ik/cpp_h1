#ifndef H_H1_CALC_60138e3b2fd242dc63716d63350c66e9
#define H_H1_CALC_60138e3b2fd242dc63716d63350c66e9

#include "./exceptions.hpp"
#include "h1.hpp"
#include "h1/common.hpp"

#include <cmath>
#include <optional>

namespace h1::calc
{

class ISimpleCalculator
{
  public:
    using InputValue = int;
    using ReturnValue = double;

    // Constructors
    ISimpleCalculator() = default;
    ISimpleCalculator(ISimpleCalculator&&) = default;
    ISimpleCalculator(const ISimpleCalculator&) = default;

    // Destructor
    virtual ~ISimpleCalculator() H1_NOEXCEPT = default;

    // Copy/Move operations
    ISimpleCalculator& operator=(const ISimpleCalculator&) = default;
    ISimpleCalculator& operator=(ISimpleCalculator&&) = default;

    // Methods
    H1_NODISCARD virtual ReturnValue calculate() = 0;
};

class BaseSimpleCalculator : public ISimpleCalculator
{
  public:
    struct Task
    {
        InputValue left;
        InputValue right;
        OperationStatus status;
        ReturnValue value;
        std::optional<String> errorMessage;
    };

    explicit BaseSimpleCalculator(Task task);

    H1_NODISCARD const Task& getTask() const H1_NOEXCEPT;
    Task& getTask() H1_NOEXCEPT;

  private:
    Task task_;
};

class AdditionCalculator : public BaseSimpleCalculator
{
  public:
    using BaseSimpleCalculator::BaseSimpleCalculator;
    H1_NODISCARD ReturnValue calculate() H1_EXCEPT override;
};

class SubtractionCalculator : public BaseSimpleCalculator
{
  public:
    using BaseSimpleCalculator::BaseSimpleCalculator;
    H1_NODISCARD ReturnValue calculate() H1_EXCEPT override;
};

class MultiplicationCalculator : public BaseSimpleCalculator
{
  public:
    using BaseSimpleCalculator::BaseSimpleCalculator;
    H1_NODISCARD ReturnValue calculate() H1_EXCEPT override;
};

class DivisionCalculator : public BaseSimpleCalculator
{
  public:
    using BaseSimpleCalculator::BaseSimpleCalculator;
    H1_NODISCARD ReturnValue calculate() H1_EXCEPT override;
};

class FactorialCalculator : public BaseSimpleCalculator
{
  public:
    using BaseSimpleCalculator::BaseSimpleCalculator;
    H1_NODISCARD ReturnValue calculate() H1_EXCEPT override;
};

class PowerCalculator : public BaseSimpleCalculator
{
  public:
    using BaseSimpleCalculator::BaseSimpleCalculator;
    H1_NODISCARD ReturnValue calculate() H1_EXCEPT override;
};
} // namespace h1::calc
#endif
