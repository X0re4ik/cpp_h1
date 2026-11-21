#include "h1/io_server/handlers/task_manager.hpp"
#include "h1/io_server/request.hpp"
#include "h1/io_server/response.hpp"
#include "test_mocks.hpp"

#include <nlohmann/json.hpp>

#include <optional>
#include <string>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace
{

using h1::test::MockCalcLocalCache;
using h1::test::MockDBManager;
using testing::_;
using testing::Return;

constexpr double leftValue = 2.0;
constexpr double rightValue = 3.0;
constexpr double sumValue = 5.0;
constexpr double dividend = 10.0;
constexpr double factorialInput = 5.0;
constexpr double zeroValue = 0.0;
constexpr double factorialResult = 120.0;

h1::io_server::Response handle(const h1::io_server::HandlerPtr& handler,
                               const std::string& body)
{
    h1::io_server::Request request;
    request.method = "POST";
    request.uri = "/h1/task";
    request.body = body;
    h1::io_server::Response response;
    handler->handleRequest(request, response);
    return response;
}

h1::entity::CalculationResultEntity cachedAddition()
{
    return h1::entity::CalculationResultEntity{
        .id = 1,
        .left = 2,
        .right = 3,
        .result = sumValue,
        .operation = "+",
        .statusCode = 0,
        .errorMessage = std::nullopt,
        .isSuccess = true,
    };
}

} // namespace

TEST(TaskManagerHandler, ReturnsJsonResultOnCacheMiss)
{
    MockDBManager dbManager;
    MockCalcLocalCache cache;

    EXPECT_CALL(cache, getIfExist(_)).WillOnce(Return(std::nullopt));
    EXPECT_CALL(cache, add(_, _)).WillOnce(Return(true));
    EXPECT_CALL(dbManager, registerOk(leftValue, rightValue, sumValue, "+", _));

    auto handler = h1::io_server::makeTaskManagerHandler(dbManager, cache);
    const auto response =
        handle(handler, R"({"left": 2, "right": 3, "operation": "+"})");

    EXPECT_EQ(response.status, h1::io_server::Response::Status::Ok);
    const auto body = nlohmann::json::parse(response.content);
    EXPECT_EQ(body.at("operation"), "+");
    EXPECT_EQ(body.at("left"), 2);
    EXPECT_EQ(body.at("right"), 3);
    EXPECT_EQ(body.at("result"), sumValue);
    EXPECT_EQ(body.at("fromCache"), false);
}

TEST(TaskManagerHandler, ReturnsCachedResultWithoutCalculation)
{
    MockDBManager dbManager;
    MockCalcLocalCache cache;

    const auto entity = cachedAddition();
    EXPECT_CALL(cache, getIfExist(_))
        .WillOnce(Return(
            std::optional<h1::lcache::ICalcLocalCache::Result_t>{entity}));
    EXPECT_CALL(cache, add(_, _)).Times(0);
    EXPECT_CALL(dbManager, registerOk(_, _, _, _, _)).Times(0);
    EXPECT_CALL(dbManager, registerError(_, _, _, _, _)).Times(0);

    auto handler = h1::io_server::makeTaskManagerHandler(dbManager, cache);
    const auto response =
        handle(handler, R"({"left": 2, "right": 3, "operation": "+"})");

    EXPECT_EQ(response.status, h1::io_server::Response::Status::Ok);
    const auto body = nlohmann::json::parse(response.content);
    EXPECT_EQ(body.at("result"), sumValue);
    EXPECT_EQ(body.at("fromCache"), true);
}

TEST(TaskManagerHandler, ReturnsJsonErrorForCalculationFailure)
{
    MockDBManager dbManager;
    MockCalcLocalCache cache;

    EXPECT_CALL(cache, getIfExist(_)).WillOnce(Return(std::nullopt));
    EXPECT_CALL(cache, add(_, _)).WillOnce(Return(true));
    EXPECT_CALL(dbManager, registerError(dividend, zeroValue, "/", _, _));

    auto handler = h1::io_server::makeTaskManagerHandler(dbManager, cache);
    const auto response =
        handle(handler, R"({"left": 10, "right": 0, "operation": "/"})");

    EXPECT_EQ(response.status,
              h1::io_server::Response::Status::InternalServerError);
    const auto body = nlohmann::json::parse(response.content);
    EXPECT_FALSE(body.at("error").get<std::string>().empty());
}

TEST(TaskManagerHandler, ReturnsJsonErrorForInvalidJson)
{
    MockDBManager dbManager;
    MockCalcLocalCache cache;

    EXPECT_CALL(cache, getIfExist(_)).Times(0);
    EXPECT_CALL(cache, add(_, _)).Times(0);
    EXPECT_CALL(dbManager, registerOk(_, _, _, _, _)).Times(0);
    EXPECT_CALL(dbManager, registerError(_, _, _, _, _)).Times(0);

    auto handler = h1::io_server::makeTaskManagerHandler(dbManager, cache);
    const auto response = handle(handler, "not-json");

    EXPECT_EQ(response.status,
              h1::io_server::Response::Status::InternalServerError);
    const auto body = nlohmann::json::parse(response.content);
    EXPECT_FALSE(body.at("error").get<std::string>().empty());
}

TEST(TaskManagerHandler, ReturnsFactorialResultWithValueField)
{
    MockDBManager dbManager;
    MockCalcLocalCache cache;

    EXPECT_CALL(cache, getIfExist(_)).WillOnce(Return(std::nullopt));
    EXPECT_CALL(cache, add(_, _)).WillOnce(Return(true));
    EXPECT_CALL(dbManager,
                registerOk(factorialInput, zeroValue, factorialResult, "!", _));

    auto handler = h1::io_server::makeTaskManagerHandler(dbManager, cache);
    const auto response = handle(handler, R"({"value": 5, "operation": "!"})");

    EXPECT_EQ(response.status, h1::io_server::Response::Status::Ok);
    const auto body = nlohmann::json::parse(response.content);
    EXPECT_EQ(body.at("operation"), "!");
    EXPECT_EQ(body.at("value"), 5);
    EXPECT_EQ(body.at("result"), factorialResult);
    EXPECT_FALSE(body.contains("left"));
    EXPECT_FALSE(body.contains("right"));
}
