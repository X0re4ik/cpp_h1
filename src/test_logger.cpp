#include "h1/logger.hpp"

#include <array>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

TEST(TestLogger, CodeLocationGetter)
{
    // Arrange
    auto codeLoc = h1_logger::CodeLocation(1, "2", "3");

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
void runLoggerInThread(const char* symbol, int count)
{
    for (int i = 0; i < count; ++i)
    {
        LOG_INFO(symbol);
    }
}

} // namespace

TEST(TestLogger, WorkLoggerInSeveralThreads)
{
    // Arrange
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
