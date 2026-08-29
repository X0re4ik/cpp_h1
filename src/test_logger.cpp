#include "h1/logger/logger.hpp"

#include <array>
#include <string>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

TEST(TestLogger, CodeLocationGetter)
{
    // Arrange
    auto codeLoc = h1::log::CodeLocation(1, "2", "3");

    // Act
    auto line = codeLoc.getlineIn();
    decltype(auto) filename = codeLoc.getFilenameIn();
    decltype(auto) funcName = codeLoc.getFunctionIn();

    // Assert
    ASSERT_EQ(line, 1);
    ASSERT_EQ(filename, "2");
    ASSERT_EQ(funcName, "3");
}

namespace
{

void initProjectLogger()
{

    auto logger =
        h1::log::LoggerConfig().addConsoleLog().setLogName("IUCH").setPattern(
            "[%#] %v");
    h1::log::initLoggerConfig(logger);
}
void runLoggerInThread(const char* symbol, int count)
{
    for (int i = 0; i < count; ++i)
    {
        H1_LOG_INFO(symbol);
    }
}

} // namespace

TEST(TestLogger, WorkLoggerInSeveralThreads)
{
    // Arrange
    initProjectLogger();
    testing::internal::CaptureStdout();

    constexpr int countThreads = 10;
    constexpr int countCalls = 5'000;
    const std::array<const char*, countThreads> symbols = {
        "A", "B", "C", "D", "E", "F", "G", "H", "I", "J"};
    ASSERT_EQ(symbols.size(), countThreads);

    std::vector<std::thread> threads;
    threads.reserve(countThreads + 2);
    for (int i = 0; i < countThreads; ++i)
    {
        threads.emplace_back(
            std::thread(runLoggerInThread, symbols.at(i), countCalls));
    }

    // Act
    for (auto& thread : threads)
    {
        thread.join();
    }
    testing::internal::GetCapturedStdout();

    // Assert
    // Stable work without segmentation fault
}

namespace
{

std::vector<std::string> splitString(const std::string& str,
                                     const std::string& delimiter)
{
    std::vector<std::string> tokens;
    size_t start = 0;
    size_t end = 0;

    while ((end = str.find(delimiter, start)) != std::string::npos)
    {
        std::string token = str.substr(start, end - start);
        if (!token.empty())
        {
            tokens.push_back(token);
        }
        start = end + delimiter.length();
    }

    // Последний кусок
    std::string last = str.substr(start);
    if (!last.empty())
    {
        tokens.push_back(last);
    }

    return tokens;
}

} // namespace

TEST(TestLogger, DiffLineNumberInOutput)
{
    // Arrange
    testing::internal::CaptureStdout();
    initProjectLogger();

    // Act
    constexpr int lineTest1 = __LINE__ + 1;
    H1_LOG_INFO("Test #1"); // Line is 118
                            // Expected: [118] Test #1
    constexpr int lineTest2 = __LINE__ + 1;
    H1_LOG_INFO("Test #2"); // Line is 121
    // Expected: [106] Test #2

    // Assert
    auto outStr = testing::internal::GetCapturedStdout();
    auto strLines = splitString(outStr, "\n");

    constexpr int countLines = 2;
    const std::array<int, countLines> linesNumbers = {lineTest1, lineTest2};
    ASSERT_EQ(strLines.size(), linesNumbers.size());

    for (int i = 0; i < countLines; i++)
    {
        auto lineNumber = linesNumbers.at(i);
        auto lineValueInLog = strLines.at(i);

        auto pattern = "[" + std::to_string(lineNumber) + "]";
        auto isValid = lineValueInLog.find(pattern) != std::string::npos;
        ASSERT_TRUE(isValid);
    }
}
