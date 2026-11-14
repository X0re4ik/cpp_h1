#include "h1/io_server/connection.hpp"

#include "h1/io_server/connection_manager.hpp"
#include "h1/io_server/handler.hpp"
#include "h1/io_server/response.hpp"
#include "h1/io_server/router.hpp"
#include "h1/logger/logger.hpp"

#include <exception>
#include <memory>
#include <string>

namespace h1::io_server
{

void handler(const HandlerPtr& handlerPtr, const Request& request,
             Response& response)
{
    try
    {
        handlerPtr->handleRequest(request, response);
    }
    catch (const std::exception& ext)
    {
        response =
            Response::stockResponse(Response::Status::InternalServerError);
    }
}

ConnectionPtr makeConnection(boost::asio::io_context& ioContext,
                             ConnectionManager& manager, APIRouter& apiRouter)
{
    return std::make_shared<Connection>(ioContext, manager, apiRouter);
}

Connection::Connection(boost::asio::io_context& ioContext,
                       ConnectionManager& manager, APIRouter& apiRouter) :
    socket_(ioContext),
    connectionManager_(manager),
    //
    buffer_({}),
    //
    strand_(ioContext),
    //
    apiRouter_(apiRouter)
{}

void Connection::start()
{
    socket_.async_read_some(

        boost::asio::buffer(buffer_),
        strand_.wrap([self = shared_from_this()](
                         const boost::system::error_code& ext,
                         std::size_t bytes) { self->handleRead(ext, bytes); }));
}

void Connection::stop()
{
    socket_.close();
}

void Connection::handleRead(const boost::system::error_code& ext,
                            std::size_t bytesTransferred)
{
    if (ext && ext != boost::asio::error::operation_aborted)
    {
        connectionManager_.stop(shared_from_this());
        return;
    }
    if (ext)
    {
        return;
    }

    boost::tribool result;
    boost::tie(result, boost::tuples::ignore) = requestParser_.parse(
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        request_, buffer_.data(), buffer_.data() + bytesTransferred);

    if (result)
    {

        // Найти и вызвать handler для маршрута
        const auto route = apiRouter_.getHandler(request_.method, request_.uri);

        switch (route.match)
        {
            case RouteMatch::Found:
                handler(route.handler, request_, response_);
                break;
            case RouteMatch::MethodNotAllowed:
                response_ =
                    Response::stockResponse(Response::Status::MethodNotAllowed);
                break;
            case RouteMatch::NotFound:
                response_ = Response::stockResponse(Response::Status::NotFound);
                break;
        }

        H1_LOG_INFO(request_.method + " - " + request_.uri + " - " +
                    std::to_string(static_cast<int>(response_.status)));

        // Отправить клиенту ответ
        boost::asio::async_write(
            socket_, response_.toBuffers(),
            strand_.wrap([self = shared_from_this()](
                             const boost::system::error_code& ext,
                             std::size_t /*bytes_transferred*/) {
                self->handleWrite(ext);
            }));
    }
    else if (!result)
    {
        H1_LOG_INFO(request_.method + " - " + request_.uri);
        response_ = Response::stockResponse(Response::Status::BadRequest);
        boost::asio::async_write(
            socket_, response_.toBuffers(),
            strand_.wrap([self = shared_from_this()](
                             const boost::system::error_code& ext,
                             std::size_t /*bytes_transferred*/) {
                self->handleWrite(ext);
            }));
    }
    else
    {
        socket_.async_read_some(
            boost::asio::buffer(buffer_),
            strand_.wrap(
                [self = shared_from_this()](
                    const boost::system::error_code& ext, std::size_t bytes) {
                    self->handleRead(ext, bytes);
                }));
    }
}

void Connection::handleWrite(const boost::system::error_code& ext)
{
    if (!ext)
    {
        boost::system::error_code ignoredEc;
        socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both,
                         ignoredEc);
    }

    if (ext != boost::asio::error::operation_aborted)
    {
        connectionManager_.stop(shared_from_this());
    }
}

boost::asio::ip::tcp::socket& Connection::socket()
{
    return socket_;
}

} // namespace h1::io_server