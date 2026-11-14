#pragma once
#ifndef H_H1_IO_SERVER_4dfb385cdc2c5adf639ea04b7b39a9c0
#define H_H1_IO_SERVER_4dfb385cdc2c5adf639ea04b7b39a9c0

#include "h1/common.hpp"
#include "h1/io_server/connection.hpp"
#include "h1/io_server/connection_manager.hpp"
#include "h1/io_server/router.hpp"

#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/noncopyable.hpp>

#include <mutex>

namespace h1::io_server
{

class IOServer : private boost::noncopyable
{
  public:
    explicit IOServer(const std::string& address, const std::string& port,
                      APIRouter apiRouter, std::size_t countTreads);

    /// Run the server's io_service loop.
    void run();

  private:
    /// Асинхронное начало соединения
    void startAccept();

    /// Асинхронное завершение соединения
    void handleAccept(const boost::system::error_code& exc);

    /// Остановка сервера
    void handleStop();

    /// The io_service used to perform asynchronous operations.
    boost::asio::io_context ioContext_;

    /// The signal_set is used to register for process termination notifications.
    boost::asio::signal_set signals_;

    /// Acceptor used to listen for incoming connections.
    boost::asio::ip::tcp::acceptor acceptor_;

    ConnectionManager connectionManager_;
    ConnectionPtr connectionPtr;

    std::size_t countTreads_;
    APIRouter apiRouter_;
};

} // namespace h1::io_server

#endif
