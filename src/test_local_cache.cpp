#include "h1/local_cache/local_cache.hpp"

#include <gtest/gtest.h>

namespace
{
h1::entity::CalculationResultEntity result(double value)
{
    return h1::entity::CalculationResultEntity{
        .id = 1,
        .left = 1,
        .right = 2,
        .result = value,
        .operation = "+",
        .statusCode = 0,
        .errorMessage = std::nullopt,
        .isSuccess = true};
}
} // namespace

TEST(CalcLocalCache, TreatsCommutativeOperandsAsTheSameOperation)
{
    h1::lcache::CalcLocalCache cache;
    const auto first = h1::lcache::OpTask{.left = 1, .right = 2, .operation = "+"};
    const auto reversed = h1::lcache::OpTask{.left = 2, .right = 1, .operation = "+"};

    EXPECT_TRUE(cache.add(first, result(3)));
    EXPECT_FALSE(cache.add(reversed, result(3)));
    ASSERT_TRUE(cache.getIfExist(reversed).has_value());
    EXPECT_EQ(cache.get(reversed).result, 3);
}

TEST(CalcLocalCache, PreservesOperandOrderForNonCommutativeOperations)
{
    h1::lcache::CalcLocalCache cache;
    const auto first = h1::lcache::OpTask{.left = 1, .right = 2, .operation = "-"};
    const auto reversed = h1::lcache::OpTask{.left = 2, .right = 1, .operation = "-"};

    EXPECT_TRUE(cache.add(first, result(-1)));
    EXPECT_FALSE(cache.getIfExist(reversed).has_value());
}

TEST(CalcLocalCache, ClearRemovesAllEntries)
{
    h1::lcache::CalcLocalCache cache;
    cache.add({.left = 1, .right = 2, .operation = "*"}, result(2));
    cache.clear();
    EXPECT_TRUE(cache.values().empty());
}
