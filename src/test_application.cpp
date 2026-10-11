#include "h1/application.hpp"
#include "h1/common.hpp"

#include <sstream>
#include <string>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace
{

std::vector<char*> makeArgv(std::vector<std::string>& args)
{
    std::vector<char*> argv;
    argv.reserve(args.size());

    for (auto& arg : args)
    {
        argv.push_back(arg.data());
    }

    return argv;
}

class MockDBManager : public h1::db::IDBManager
{
  public:
    MOCK_METHOD(void, registerOk,
                (Value_t, Value_t, Value_t, const h1::String&, int),
                (override));

    MOCK_METHOD(void, registerError,
                (Value_t, Value_t, const h1::String&, int, const h1::String&),
                (override));

    MOCK_METHOD(std::vector<h1::entity::CalculationResultEntity>, getData,
                (int), (override));
};

class MockCalcLocalCache : public h1::lcache::ICalcLocalCache
{
  public:
    MOCK_METHOD(void, clear, (), (override));

    MOCK_METHOD(std::vector<Result_t>, values, (), (const, override));

    MOCK_METHOD(bool, add, (const h1::lcache::OpTask&, Result_t), (override));

    MOCK_METHOD(Result_t, get, (const h1::lcache::OpTask&), (const, override));

    MOCK_METHOD(std::optional<Result_t>, getIfExist,
                (const h1::lcache::OpTask&), (const, override));
};

TEST(TestApplication, PrintsResultForValidAdditionJson)
{
    // Arrange
    testing::internal::CaptureStdout();
    testing::NiceMock<MockDBManager> mockDBManager;
    testing::NiceMock<MockCalcLocalCache> mockCalcLocalCache;
    std::ostringstream out;
    std::ostringstream err;
    h1::Application app(mockDBManager, mockCalcLocalCache, out, err);

    std::vector<std::string> args = {
        "h1",
        "--config",
        R"({"left": 1, "right": 1, "operation": "+"})",
    };
    auto argv = makeArgv(args);

    // Act
    app.run(static_cast<int>(argv.size()), argv.data());

    // Assert
    testing::internal::GetCapturedStdout();
    EXPECT_EQ(out.str(), "1+1=2.00\n");
    EXPECT_TRUE(err.str().empty());
}

TEST(TestApplication, PrintsResultForInvalidAdditionJson)
{
    // Arrange
    testing::internal::CaptureStdout();
    std::ostringstream out;
    std::ostringstream err;
    testing::NiceMock<MockDBManager> mockDBManager;
    testing::NiceMock<MockCalcLocalCache> mockCalcLocalCache;
    h1::Application app(mockDBManager, mockCalcLocalCache, out, err);

    std::vector<std::string> args = {
        "h1",
        "--config",
        R"({"value1": 1, "operation": "!"})",
    };
    auto argv = makeArgv(args);

    // Act
    app.run(static_cast<int>(argv.size()), argv.data());

    // Assert
    testing::internal::GetCapturedStdout();
    EXPECT_TRUE(out.str().empty());
    EXPECT_EQ(err.str(), "Неизветный ключ 'value1'\n");
}

} // namespace
