#include "h1/io_server/request_parser.hpp"

#include <gtest/gtest.h>

#include <string>
#include <utility>

namespace
{

using boost::indeterminate;
using boost::tribool;
using h1::io_server::Request;
using h1::io_server::RequestParser;

/// Скармливает данные парсеру по одному байту — как при частичном
/// чтении из сокета. Возвращает true, если запрос разобран полностью.
tribool parseAll(const std::string& data, Request& req)
{
    RequestParser parser;
    tribool result = indeterminate;
    for (char c : data)
    {
        auto [r, ignored] = parser.parse(req, &c, &c + 1);
        (void)ignored;
        result = r;
        if (r || !r)
        {
            return result;
        }
    }
    return result;
}

tribool parseInChunks(const std::string& data, Request& req, std::size_t chunkSize)
{
    RequestParser parser;
    tribool result = indeterminate;
    for (std::size_t offset = 0; offset < data.size(); offset += chunkSize)
    {
        const auto begin = data.begin() + static_cast<long>(offset);
        const auto end = (offset + chunkSize < data.size())
                             ? begin + static_cast<long>(chunkSize)
                             : data.end();
        auto [r, ignored] = parser.parse(req, begin, end);
        (void)ignored;
        result = r;
        if (r || !r)
        {
            return result;
        }
    }
    return result;
}

TEST(RequestParserTest, GetWithoutBodyIsComplete)
{
    Request req;
    const auto result = parseAll(
        "GET /h1/test/ HTTP/1.1\r\nHost: localhost\r\n\r\n", req);

    EXPECT_TRUE(result);
    EXPECT_EQ(req.method, "GET");
    EXPECT_EQ(req.uri, "/h1/test/");
    EXPECT_TRUE(req.body.empty());
}

TEST(RequestParserTest, PostWithBodyIsComplete)
{
    Request req;
    const auto result = parseAll(
        "POST /h1/calc HTTP/1.1\r\n"
        "Content-Length: 13\r\n"
        "Content-Type: text/plain\r\n"
        "\r\n"
        "hello, world!",
        req);

    EXPECT_TRUE(result);
    EXPECT_EQ(req.method, "POST");
    EXPECT_EQ(req.body, "hello, world!");
}

TEST(RequestParserTest, BodySplitAcrossChunksIsAssembled)
{
    Request req;
    const std::string data =
        "POST /h1/calc HTTP/1.1\r\nContent-Length: 5\r\n\r\nheade";

    // По 3 байта за раз
    EXPECT_TRUE(parseInChunks(data, req, 3));
    EXPECT_EQ(req.body, "heade");
    EXPECT_EQ(req.body.size(), 5);
}

TEST(RequestParserTest, ContentLengthHeaderIsCaseInsensitive)
{
    Request req;
    const auto result = parseAll(
        "POST / HTTP/1.1\r\ncontent-length: 2\r\n\r\nhi", req);

    EXPECT_TRUE(result);
    EXPECT_EQ(req.body, "hi");
}

TEST(RequestParserTest, InvalidContentLengthFails)
{
    Request req;
    const auto result = parseAll(
        "POST / HTTP/1.1\r\nContent-Length: abc\r\n\r\n", req);

    EXPECT_FALSE(result);
}

TEST(RequestParserTest, ResetClearsBodyState)
{
    Request req;
    RequestParser parser;
    const std::string data =
        "POST / HTTP/1.1\r\nContent-Length: 5\r\n\r\nhello";

    // Первый запрос — целиком
    auto [first, ignored1] = parser.parse(req, data.begin(), data.end());
    (void)ignored1;
    EXPECT_TRUE(first);
    EXPECT_EQ(req.body, "hello");

    // После reset парсер готов к новому запросу без тела
    parser.reset();
    Request next;
    const auto second = parseAll("GET / HTTP/1.1\r\n\r\n", next);
    EXPECT_TRUE(second);
    EXPECT_TRUE(next.body.empty());
}

} // namespace
