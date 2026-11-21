#pragma once
#ifndef H_H1_IO_SERVER_CONNECTION_7f0e3e66e9abe63954a43665bf726036
#define H_H1_IO_SERVER_CONNECTION_7f0e3e66e9abe63954a43665bf726036

#include "h1/common.hpp"
#include "h1/io_server/request.hpp"
#include "h1/io_server/request_parser.hpp"
#include "h1/io_server/response.hpp"
#include "h1/io_server/router.hpp"

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

#include <memory>

constexpr int gBufferSize = 64 * 256;

namespace h1::io_server
{

class ConnectionManager;

class Connection :
    public std::enable_shared_from_this<Connection>,
    private boost::noncopyable
{
  public:
    explicit Connection(boost::asio::io_context& ioContext,
                        ConnectionManager& manager, APIRouter& apiRouter);

    boost::asio::ip::tcp::socket& socket();

    // Запустить асинхронную операцию над соединением
    void start();

    // Остановить все асинхронные операции над соединением
    void stop();

  private:
    // Callback на чтение данных
    void handleRead(const boost::system::error_code&, std::size_t);
    // Callback на запись данных
    void handleWrite(const boost::system::error_code&);

    boost::asio::ip::tcp::socket socket_;
    ConnectionManager& connectionManager_;
    boost::array<char, gBufferSize> buffer_;

    boost::asio::io_context::strand strand_;

    Request request_;
    Response response_;

    APIRouter& apiRouter_;

    RequestParser requestParser_;
};

using ConnectionPtr = std::shared_ptr<Connection>;

ConnectionPtr makeConnection(boost::asio::io_context& ioContext,
                             ConnectionManager& manager, APIRouter& apiRouter);

} // namespace h1::io_server

#endif
