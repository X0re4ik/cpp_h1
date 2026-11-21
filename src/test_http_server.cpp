#include "h1/io_server/handlers/task_manager.hpp"
#include "h1/io_server/io_server.hpp"
#include "h1/io_server/router.hpp"
#include "test_mocks.hpp"

#include <boost/asio.hpp>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <thread>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace
{

using h1::io_server::APIRouter;
using h1::io_server::IOServer;
using h1::io_server::makeTaskManagerHandler;
using h1::test::MockCalcLocalCache;
using h1::test::MockDBManager;
using testing::_;
using testing::Return;

using boost::asio::ip::tcp;

constexpr int httpOk = 200;
constexpr int httpInternalError = 500;
constexpr double leftValue = 2.0;
constexpr double rightValue = 3.0;
constexpr double sumValue = 5.0;
constexpr double dividend = 10.0;
constexpr double zeroValue = 0.0;
constexpr double factorialInput = 5.0;
constexpr double factorialResult = 120.0;

struct HttpResponse
{
    int status{0};
    std::map<std::string, std::string> headers;
    std::string body;
};

std::string toLower(std::string value)
{
    for (auto& character : value)
    {
        character = static_cast<char>(
            std::tolower(static_cast<unsigned char>(character)));
    }
    return value;
}

HttpResponse parseResponse(const std::string& raw)
{
    const auto headerEnd = raw.find("\r\n\r\n");
    if (headerEnd == std::string::npos)
    {
        throw std::runtime_error("invalid HTTP response: no header terminator");
    }

    const auto statusLineEnd = raw.find("\r\n");
    const auto firstSpace = raw.find(' ');
    if (statusLineEnd == std::string::npos || firstSpace == std::string::npos)
    {
        throw std::runtime_error("invalid HTTP response: no status line");
    }

    HttpResponse response;
    response.status = std::stoi(raw.substr(firstSpace + 1, 3));

    std::size_t position = statusLineEnd + 2;
    while (position < headerEnd)
    {
        const auto lineEnd = raw.find("\r\n", position);
        if (lineEnd == std::string::npos || lineEnd > headerEnd)
        {
            break;
        }

        const auto line = raw.substr(position, lineEnd - position);
        const auto colon = line.find(':');
        if (colon != std::string::npos)
        {
            auto value = line.substr(colon + 1);
            if (!value.empty() && value.front() == ' ')
            {
                value.erase(0, 1);
            }
            response.headers[toLower(line.substr(0, colon))] = value;
        }
        position = lineEnd + 2;
    }

    response.body = raw.substr(headerEnd + 4);
    return response;
}

HttpResponse postJson(std::uint16_t port, const std::string& json)
{
    boost::asio::io_context context;
    tcp::socket socket(context);
    socket.connect({boost::asio::ip::address_v4::loopback(), port});

    const std::string request = "POST /h1/task HTTP/1.1\r\n"
                                "Host: localhost\r\n"
                                "Content-Type: application/json\r\n"
                                "Content-Length: " +
                                std::to_string(json.size()) + "\r\n" +
                                "Connection: close\r\n\r\n" + json;

    boost::asio::write(socket, boost::asio::buffer(request));

    boost::asio::streambuf buffer;
    boost::system::error_code error;
    boost::asio::read(socket, buffer, error);

    const std::string raw(boost::asio::buffers_begin(buffer.data()),
                          boost::asio::buffers_end(buffer.data()));
    return parseResponse(raw);
}

h1::entity::CalculationResultEntity cachedAddition()
{
    return h1::entity::CalculationResultEntity{
        .id = 1,
        .left = leftValue,
        .right = rightValue,
        .result = sumValue,
        .operation = "+",
        .statusCode = 0,
        .errorMessage = std::nullopt,
        .isSuccess = true,
    };
}

class HttpServerTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        auto router = APIRouter("/h1");
        router.registerHandler("POST", "/task",
                               makeTaskManagerHandler(dbManager_, cache_));
        server_ = std::make_unique<IOServer>("127.0.0.1", "0",
                                             std::move(router), 4, false);
        serverThread_ = std::thread([this]() { server_->run(); });
        waitUntilReady();
    }

    void TearDown() override
    {
        if (server_)
        {
            server_->stop();
        }
        if (serverThread_.joinable())
        {
            serverThread_.join();
        }
    }

    void waitUntilReady() const
    {
        const auto endpoint = tcp::endpoint(
            boost::asio::ip::address_v4::loopback(), server_->port());
        const auto deadline =
            std::chrono::steady_clock::now() + std::chrono::seconds(3);

        while (std::chrono::steady_clock::now() < deadline)
        {
            boost::system::error_code error;
            boost::asio::io_context probeContext;
            tcp::socket probe(probeContext);
            // NOLINTNEXTLINE(bugprone-unused-return-value)
            probe.connect(endpoint, error);
            if (!error)
            {
                boost::system::error_code ignored;
                // NOLINTNEXTLINE(bugprone-unused-return-value)
                probe.close(ignored);
                return;
            }
            std::this_thread::yield();
        }

        FAIL() << "server did not start listening on port " << server_->port();
    }

    HttpResponse post(const std::string& json) const
    {
        return postJson(server_->port(), json);
    }

    // NOLINTBEGIN(cppcoreguidelines-non-private-member-variables-in-classes, misc-non-private-member-variables-in-classes)
    MockDBManager dbManager_;
    MockCalcLocalCache cache_;
    std::unique_ptr<IOServer> server_;
    std::thread serverThread_;
    // NOLINTEND(cppcoreguidelines-non-private-member-variables-in-classes, misc-non-private-member-variables-in-classes)
};

} // namespace

TEST_F(HttpServerTest, PostAdditionReturnsJsonResult)
{
    EXPECT_CALL(cache_, getIfExist(_)).WillOnce(Return(std::nullopt));
    EXPECT_CALL(cache_, add(_, _)).WillOnce(Return(true));
    EXPECT_CALL(dbManager_,
                registerOk(leftValue, rightValue, sumValue, "+", _));

    const auto response = post(R"({"left": 2, "right": 3, "operation": "+"})");

    EXPECT_EQ(response.status, httpOk);
    EXPECT_EQ(response.headers.at("content-type"), "application/json");

    const auto body = nlohmann::json::parse(response.body);
    EXPECT_EQ(body.at("operation"), "+");
    EXPECT_EQ(body.at("result"), sumValue);
    EXPECT_EQ(body.at("fromCache"), false);
}

TEST_F(HttpServerTest, PostFactorialReturnsValueAndResult)
{
    EXPECT_CALL(cache_, getIfExist(_)).WillOnce(Return(std::nullopt));
    EXPECT_CALL(cache_, add(_, _)).WillOnce(Return(true));
    EXPECT_CALL(dbManager_,
                registerOk(factorialInput, zeroValue, factorialResult, "!", _));

    const auto response = post(R"({"value": 5, "operation": "!"})");

    EXPECT_EQ(response.status, httpOk);
    const auto body = nlohmann::json::parse(response.body);
    EXPECT_EQ(body.at("operation"), "!");
    EXPECT_EQ(body.at("value"), factorialInput);
    EXPECT_EQ(body.at("result"), factorialResult);
}

TEST_F(HttpServerTest, PostCachedOperationReturnsFromCache)
{
    const auto cached = cachedAddition();
    EXPECT_CALL(cache_, getIfExist(_))
        .WillOnce(Return(
            std::optional<h1::lcache::ICalcLocalCache::Result_t>{cached}));
    EXPECT_CALL(cache_, add(_, _)).Times(0);
    EXPECT_CALL(dbManager_, registerOk(_, _, _, _, _)).Times(0);
    EXPECT_CALL(dbManager_, registerError(_, _, _, _, _)).Times(0);

    const auto response = post(R"({"left": 2, "right": 3, "operation": "+"})");

    EXPECT_EQ(response.status, httpOk);
    const auto body = nlohmann::json::parse(response.body);
    EXPECT_EQ(body.at("result"), sumValue);
    EXPECT_EQ(body.at("fromCache"), true);
}

TEST_F(HttpServerTest, PostDivisionByZeroReturnsJsonError)
{
    EXPECT_CALL(cache_, getIfExist(_)).WillOnce(Return(std::nullopt));
    EXPECT_CALL(cache_, add(_, _)).WillOnce(Return(true));
    EXPECT_CALL(dbManager_, registerError(dividend, zeroValue, "/", _, _));

    const auto response = post(R"({"left": 10, "right": 0, "operation": "/"})");

    EXPECT_EQ(response.status, httpInternalError);
    const auto body = nlohmann::json::parse(response.body);
    EXPECT_FALSE(body.at("error").get<std::string>().empty());
}

TEST_F(HttpServerTest, PostInvalidJsonReturnsJsonError)
{
    EXPECT_CALL(cache_, getIfExist(_)).Times(0);
    EXPECT_CALL(cache_, add(_, _)).Times(0);

    const auto response = post("not-json");

    EXPECT_EQ(response.status, httpInternalError);
    const auto body = nlohmann::json::parse(response.body);
    EXPECT_FALSE(body.at("error").get<std::string>().empty());
}
