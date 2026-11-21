#include "h1/io_server/handler.hpp"
#include "h1/io_server/request.hpp"
#include "h1/io_server/response.hpp"
#include "h1/io_server/router.hpp"

#include <memory>

#include <gtest/gtest.h>

namespace
{

class StubHandler : public h1::io_server::Handler
{
  public:
    void handleRequest(const h1::io_server::Request& /*request*/,
                       h1::io_server::Response& /*response*/) const override
    {}
};

h1::io_server::HandlerPtr makeStubHandler()
{
    return std::make_shared<StubHandler>();
}

} // namespace

TEST(APIRouter, FindsRegisteredRoute)
{
    h1::io_server::APIRouter router("/h1");
    router.registerHandler("POST", "/task", makeStubHandler());

    const auto result = router.getHandler("POST", "/h1/task");

    EXPECT_EQ(result.match, h1::io_server::RouteMatch::Found);
    EXPECT_NE(result.handler, nullptr);
}

TEST(APIRouter, ReturnsMethodNotAllowedForKnownPath)
{
    h1::io_server::APIRouter router("/h1");
    router.registerHandler("POST", "/task", makeStubHandler());

    const auto result = router.getHandler("GET", "/h1/task");

    EXPECT_EQ(result.match, h1::io_server::RouteMatch::MethodNotAllowed);
    EXPECT_EQ(result.handler, nullptr);
}

TEST(APIRouter, ReturnsNotFoundForUnknownPath)
{
    h1::io_server::APIRouter router("/h1");
    router.registerHandler("POST", "/task", makeStubHandler());

    const auto result = router.getHandler("POST", "/h1/unknown");

    EXPECT_EQ(result.match, h1::io_server::RouteMatch::NotFound);
    EXPECT_EQ(result.handler, nullptr);
}

TEST(APIRouter, NormalizesQueryStringAndTrailingSlash)
{
    h1::io_server::APIRouter router("/h1");
    router.registerHandler("POST", "/task", makeStubHandler());

    const auto result = router.getHandler("POST", "/h1/task/?left=1&right=2");

    EXPECT_EQ(result.match, h1::io_server::RouteMatch::Found);
}

TEST(APIRouter, NormalizesBasePathOnRegistration)
{
    h1::io_server::APIRouter router("/h1");
    router.registerHandler("POST", "/h1/task/", makeStubHandler());

    const auto result = router.getHandler("POST", "/task");

    EXPECT_EQ(result.match, h1::io_server::RouteMatch::Found);
}
