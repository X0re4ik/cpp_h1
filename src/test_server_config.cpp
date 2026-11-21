#include "h1/exceptions.hpp"
#include "h1/io_server/server_config.hpp"

#include <cstdlib>
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

h1::io_server::ServerConfig parse(std::vector<std::string> args)
{
    auto argv = makeArgv(args);
    return h1::io_server::ServerConfig::parse(static_cast<int>(argv.size()),
                                              argv.data());
}

void setEnv(const char* name, const char* value)
{
    // NOLINTNEXTLINE(concurrency-mt-unsafe)
    ASSERT_EQ(setenv(name, value, 1), 0);
}

void unsetEnv(const char* name)
{
    // NOLINTNEXTLINE(concurrency-mt-unsafe)
    unsetenv(name);
}

void clearServerEnv()
{
    unsetEnv("H1_SERVER_HOST");
    unsetEnv("H1_SERVER_PORT");
    unsetEnv("H1_SERVER_THREADS");
}

class ServerConfigTest : public testing::Test
{
  protected:
    void SetUp() override
    {
        clearServerEnv();
    }

    void TearDown() override
    {
        clearServerEnv();
    }
};

} // namespace

TEST_F(ServerConfigTest, UsesDefaultsWithoutCliAndEnv)
{
    const auto config = parse({"h1Server"});

    EXPECT_EQ(config.host, "0.0.0.0");
    EXPECT_EQ(config.port, 8080);
    EXPECT_EQ(config.threads, 4U);
    EXPECT_FALSE(config.verbose);
}

TEST_F(ServerConfigTest, ReadsEnvironment)
{
    setEnv("H1_SERVER_HOST", "127.0.0.1");
    setEnv("H1_SERVER_PORT", "7070");
    setEnv("H1_SERVER_THREADS", "2");

    const auto config = parse({"h1Server"});

    EXPECT_EQ(config.host, "127.0.0.1");
    EXPECT_EQ(config.port, 7070);
    EXPECT_EQ(config.threads, 2U);
}

TEST_F(ServerConfigTest, CliOverridesEnvironment)
{
    setEnv("H1_SERVER_HOST", "127.0.0.1");
    setEnv("H1_SERVER_PORT", "7070");
    setEnv("H1_SERVER_THREADS", "2");

    const auto config = parse(
        {"h1Server", "--host", "0.0.0.0", "--port", "9090", "--threads", "8"});

    EXPECT_EQ(config.host, "0.0.0.0");
    EXPECT_EQ(config.port, 9090);
    EXPECT_EQ(config.threads, 8U);
}

TEST_F(ServerConfigTest, ParsesVerboseFlag)
{
    const auto config = parse({"h1Server", "--verbose"});

    EXPECT_TRUE(config.verbose);
}

TEST_F(ServerConfigTest, HelpExitsWithoutParsingConfig)
{
    EXPECT_EXIT((void)parse({"h1Server", "--help"}), testing::ExitedWithCode(0),
                "");
}

TEST_F(ServerConfigTest, RejectsInvalidPort)
{
    EXPECT_THROW((void)parse({"h1Server", "--port", "70000"}),
                 h1::AppException);
}

TEST_F(ServerConfigTest, RejectsInvalidThreadCount)
{
    EXPECT_THROW((void)parse({"h1Server", "--threads", "0"}), h1::AppException);
    EXPECT_THROW((void)parse({"h1Server", "--threads", "1"}), h1::AppException);
}

TEST_F(ServerConfigTest, RejectsOneThreadFromEnvironment)
{
    setEnv("H1_SERVER_THREADS", "1");

    EXPECT_THROW((void)parse({"h1Server"}), h1::AppException);
}

TEST_F(ServerConfigTest, AcceptsTwoThreads)
{
    const auto config = parse({"h1Server", "--threads", "2"});

    EXPECT_EQ(config.threads, 2U);
}
