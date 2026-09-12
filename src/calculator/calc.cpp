#include "h1/calculator/calc.hpp"

#include "h1.hpp"
#include "h1/calculator/exceptions.hpp"
#include "task.hpp"

#include <iostream>
#include <memory>

namespace
{
struct H1TaskDeleter
{
    void operator()(Task* task) const
    {
        deleteTask(task);
    }
};

void calcOperationFromH1Library(h1::calc::BaseSimpleCalculator::Task& calcTask,
                                OperationEnum operation,
                                int (*function)(struct Task*))
{
    // Arranges
    auto task = std::unique_ptr<Task, H1TaskDeleter>(createTask());
    task->operation = operation;
    task->left = calcTask.left;
    task->right = calcTask.right;

    // Act
    function(task.get());

    calcTask.status = task->status;

    // Result
    if (task->status != OperationStatus::OK)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
        throw h1::calc::CalculatorException(task->errorMessage);
    }
    calcTask.value = task->result;
}

} // namespace

namespace h1::calc
{

// ===== [START] BaseSimpleCalculator =====
BaseSimpleCalculator::BaseSimpleCalculator(BaseSimpleCalculator::Task task) :
    task_(task)
{}

const BaseSimpleCalculator::Task&
    BaseSimpleCalculator::getTask() const H1_NOEXCEPT
{
    return task_;
}

BaseSimpleCalculator::Task& BaseSimpleCalculator::getTask() H1_NOEXCEPT
{
    return task_;
}

// ===== [START] AdditionCalculator =====
AdditionCalculator::ReturnValue AdditionCalculator::calculate()
{
    auto& task = getTask();
    calcOperationFromH1Library(task, OperationEnum::ADDITION, makeAddition);
    return task.value;
}

// ===== [START] SubtractionCalculator =====
SubtractionCalculator::ReturnValue SubtractionCalculator::calculate()
{
    auto& task = getTask();
    calcOperationFromH1Library(task, OperationEnum::SUBTRACTION,
                               makeSubtraction);
    return task.value;
}

// ===== [START] MultiplicationCalculator =====
MultiplicationCalculator::ReturnValue MultiplicationCalculator::calculate()
{
    auto& task = getTask();
    calcOperationFromH1Library(task, OperationEnum::MULTIPLICATION,
                               makeMultiplication);
    return task.value;
}

// ===== [START] DivisionCalculator =====
DivisionCalculator::ReturnValue DivisionCalculator::calculate()
{
    auto& task = getTask();
    calcOperationFromH1Library(task, OperationEnum::DIVISION, makeDivision);
    return task.value;
}

// ===== [START] FactorialCalculator =====
FactorialCalculator::ReturnValue FactorialCalculator::calculate()
{
    auto& task = getTask();
    calcOperationFromH1Library(task, OperationEnum::FACTORIAL, makeFactorial);
    return task.value;
}

// ===== [START] PowerCalculator =====
PowerCalculator::ReturnValue PowerCalculator::calculate()
{
    auto& task = getTask();
    calcOperationFromH1Library(task, OperationEnum::POWER, makePower);
    return task.value;
}

} // namespace h1::calc