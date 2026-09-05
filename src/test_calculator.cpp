#include "h1/calculator/calc.hpp"

#include <array>
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include <gtest/gtest.h>

namespace
{

// Вспомогательная функция для создания Task для бинарных операций
h1::calc::BaseSimpleCalculator::Task
    // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
    makeBinaryTask(int left, int right,
                   OperationStatus status = OperationStatus::OK)
{
    h1::calc::BaseSimpleCalculator::Task task{};
    task.left = left;
    task.right = right;
    task.status = status;
    task.value = 0.0;
    return task;
}

// Вспомогательная функция для создания Task для унарных операций
h1::calc::BaseSimpleCalculator::Task
    makeUnaryTask(int value, OperationStatus status = OperationStatus::OK)
{
    h1::calc::BaseSimpleCalculator::Task task{};
    task.left = value;
    task.right = 0;
    task.status = status;
    task.value = 0.0;
    return task;
}

struct BinaryCalculatorCase
{
    int left;
    int right;
    double expected;
};

template <typename Calculator>
class BinaryDefaultCalculatorTest :
    public testing::TestWithParam<BinaryCalculatorCase>
{};

// ==== [START] AdditionCalculatorTest ====
using AdditionCalculatorTest =
    BinaryDefaultCalculatorTest<h1::calc::AdditionCalculator>;

TEST_P(AdditionCalculatorTest, CalculatesExpectedValue)
{
    // Arrage
    const auto testCase = GetParam();
    auto task = makeBinaryTask(testCase.left, testCase.right);
    h1::calc::AdditionCalculator calculator(task);

    // Act
    auto res = calculator.calculate();

    // Assert
    ASSERT_DOUBLE_EQ(res, testCase.expected);
}

INSTANTIATE_TEST_SUITE_P(TestValidAddition, AdditionCalculatorTest,
                         testing::Values(BinaryCalculatorCase{10, 15, 25},
                                         BinaryCalculatorCase{20, 10, 30},
                                         BinaryCalculatorCase{-10, 15, 5},
                                         BinaryCalculatorCase{-20, 10, -10},
                                         BinaryCalculatorCase{0, 0, 0}));

// ==== [START] SubtractionCalculatorTest ====
using SubtractionCalculatorTest =
    BinaryDefaultCalculatorTest<h1::calc::SubtractionCalculator>;

TEST_P(SubtractionCalculatorTest, CalculatesExpectedValue)
{
    // Arrage
    const auto testCase = GetParam();
    auto task = makeBinaryTask(testCase.left, testCase.right);
    h1::calc::SubtractionCalculator calculator(task);

    // Act
    auto res = calculator.calculate();

    // Assert
    ASSERT_DOUBLE_EQ(res, testCase.expected);
}

INSTANTIATE_TEST_SUITE_P(TestValidSubtraction, SubtractionCalculatorTest,
                         testing::Values(BinaryCalculatorCase{10, 15, -5},
                                         BinaryCalculatorCase{20, 10, 10},
                                         BinaryCalculatorCase{-10, 15, -25},
                                         BinaryCalculatorCase{-20, 10, -30},
                                         BinaryCalculatorCase{0, 0, 0}));

// ==== [START] MultiplicationCalculatorTest ====
using MultiplicationCalculatorTest =
    BinaryDefaultCalculatorTest<h1::calc::MultiplicationCalculator>;

TEST_P(MultiplicationCalculatorTest, CalculatesExpectedValue)
{
    // Arrage
    const auto testCase = GetParam();
    auto task = makeBinaryTask(testCase.left, testCase.right);
    h1::calc::MultiplicationCalculator calculator(task);

    // Act
    auto res = calculator.calculate();

    // Assert
    ASSERT_DOUBLE_EQ(res, testCase.expected);
}

INSTANTIATE_TEST_SUITE_P(TestValidMultiplication, MultiplicationCalculatorTest,
                         testing::Values(BinaryCalculatorCase{10, 15, 150},
                                         BinaryCalculatorCase{20, 10, 200},
                                         BinaryCalculatorCase{-10, 15, -150},
                                         BinaryCalculatorCase{-20, 10, -200},
                                         BinaryCalculatorCase{-20, -10, 200},
                                         BinaryCalculatorCase{0, 0, 0}));

// ==== [START] DivisionCalculatorTest ====
using DivisionCalculatorTest =
    BinaryDefaultCalculatorTest<h1::calc::DivisionCalculator>;

TEST_P(DivisionCalculatorTest, CalculatesExpectedValue)
{
    // Arrage
    const auto testCase = GetParam();
    auto task = makeBinaryTask(testCase.left, testCase.right);
    h1::calc::DivisionCalculator calculator(task);

    // Act
    auto res = calculator.calculate();

    // Assert
    ASSERT_DOUBLE_EQ(res, testCase.expected);
}

INSTANTIATE_TEST_SUITE_P(TestValidDivision, DivisionCalculatorTest,
                         testing::Values(BinaryCalculatorCase{10, 5, 2},
                                         BinaryCalculatorCase{20, 10, 2},
                                         BinaryCalculatorCase{-10, 5, -2},
                                         BinaryCalculatorCase{-20, -5, 4},
                                         BinaryCalculatorCase{-20, -10, 2}));

// ==== [START] FactorialCalculatorTest ====
struct FactorialCalculatorCase
{
    int value;
    double expected;
};

template <typename Calculator>
class FactorialCalculatorCaseTest :
    public testing::TestWithParam<FactorialCalculatorCase>
{};

using FactorialCalculatorTest =
    FactorialCalculatorCaseTest<h1::calc::FactorialCalculator>;

TEST_P(FactorialCalculatorTest, CalculatesExpectedValue)
{
    // Arrage
    const auto testCase = GetParam();
    auto task = makeUnaryTask(testCase.value);
    h1::calc::FactorialCalculator calculator(task);

    // Act
    auto res = calculator.calculate();

    // Assert
    ASSERT_DOUBLE_EQ(res, testCase.expected);
}

INSTANTIATE_TEST_SUITE_P(TestValidFactorial, FactorialCalculatorTest,
                         testing::Values(FactorialCalculatorCase{10, 3628800},
                                         FactorialCalculatorCase{1, 1},
                                         FactorialCalculatorCase{0, 1}));

// ==== [START] PowerCalculatorTest ====
using PowerCalculatorTest =
    BinaryDefaultCalculatorTest<h1::calc::PowerCalculator>;

TEST_P(PowerCalculatorTest, CalculatesExpectedValue)
{
    // Arrage
    const auto testCase = GetParam();
    auto task = makeBinaryTask(testCase.left, testCase.right);
    h1::calc::PowerCalculator calculator(task);

    // Act
    auto res = calculator.calculate();

    // Assert
    ASSERT_DOUBLE_EQ(res, testCase.expected);
}

INSTANTIATE_TEST_SUITE_P(TestValidPower, PowerCalculatorTest,
                         testing::Values(BinaryCalculatorCase{1, 1, 1},
                                         BinaryCalculatorCase{5, 3, 125},
                                         BinaryCalculatorCase{10, 0, 1},
                                         BinaryCalculatorCase{7, 8, 5764801},
                                         BinaryCalculatorCase{-101, 3,
                                                              -1030301},
                                         BinaryCalculatorCase{-101, 2, 10201}));

} // namespace