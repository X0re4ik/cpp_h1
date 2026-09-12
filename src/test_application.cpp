#include "h1/application.hpp"

#include <sstream>
#include <string>
#include <vector>

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

TEST(TestApplication, PrintsResultForValidAdditionJson)
{
    // Arrange
    testing::internal::CaptureStdout();
    std::ostringstream out;
    std::ostringstream err;
    h1::Application app(out, err);

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
    h1::Application app(out, err);

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
