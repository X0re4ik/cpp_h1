#include "h1/io_server/connection.hpp"

#include "h1/io_server/connection_manager.hpp"
#include "h1/io_server/response.hpp"

#include <memory>

namespace h1::io_server
{

ConnectionPtr makeConnection(boost::asio::io_context& ioContext,
                             ConnectionManager& manager)
{
    return std::make_shared<Connection>(ioContext, manager);
}

Connection::Connection(boost::asio::io_context& ioContext,
                       ConnectionManager& manager) :
    socket_(ioContext),
    connectionManager_(manager), buffer_({})
{}

void Connection::start()
{
    socket_.async_read_some(
        boost::asio::buffer(buffer_),
        [self = shared_from_this()](const boost::system::error_code& ext,
                                    std::size_t bytes) {
            self->handleRead(ext, bytes);
        });
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
        // Вызвать handler на получение ответа от сервера
        // TODO
        response_.content = "Hello World";
        response_.status = Response::Status::Ok;

        // Отправить клиенту ответ
        boost::asio::async_write(
            socket_, response_.toBuffers(),
            [self = shared_from_this()](const boost::system::error_code& ext,
                                        std::size_t /*bytes_transferred*/) {
                self->handleWrite(ext);
            });
    }
    else if (!result)
    {
        response_ = Response::stockResponse(Response::Status::BadRequest);
        boost::asio::async_write(
            socket_, response_.toBuffers(),
            [self = shared_from_this()](const boost::system::error_code& ext,
                                        std::size_t /*bytes_transferred*/) {
                self->handleWrite(ext);
            });
    }
    else
    {
        socket_.async_read_some(
            boost::asio::buffer(buffer_),
            [self = shared_from_this()](const boost::system::error_code& ext,
                                        std::size_t bytes) {
                self->handleRead(ext, bytes);
            });
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