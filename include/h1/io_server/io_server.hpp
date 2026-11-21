#pragma once
#ifndef H_H1_IO_SERVER_4dfb385cdc2c5adf639ea04b7b39a9c0
#define H_H1_IO_SERVER_4dfb385cdc2c5adf639ea04b7b39a9c0

#include "h1/common.hpp"
#include "h1/io_server/connection.hpp"
#include "h1/io_server/connection_manager.hpp"
#include "h1/io_server/router.hpp"

#include <boost/asio.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>
#include <boost/noncopyable.hpp>

#include <atomic>
#include <cstdint>

namespace h1::io_server
{

class IOServer : private boost::noncopyable
{
  public:
    explicit IOServer(const std::string& address, const std::string& port,
                      APIRouter apiRouter, std::size_t countTreads,
                      bool enableSignals = true);

    /// Run the server's io_service loop.
    void run();

    /// Остановить сервер: закрыть acceptor, соединения и io_context.
    void stop();

    /// Фактический порт, на котором слушает сервер (полезно при порту 0).
    H1_NODISCARD std::uint16_t port() const;

  private:
    /// Асинхронное начало соединения
    void startAccept();

    /// Асинхронное завершение соединения
    void handleAccept(const boost::system::error_code& exc,
                      const ConnectionPtr& connection);

    /// Остановка сервера
    void handleStop();

    /// The io_service used to perform asynchronous operations.
    boost::asio::io_context ioContext_;

    /// The signal_set is used to register for process termination notifications.
    boost::asio::signal_set signals_;

    /// Acceptor used to listen for incoming connections.
    boost::asio::ip::tcp::acceptor acceptor_;

    /// Strand, сериализующий accept/close операции acceptor-а.
    boost::asio::strand<boost::asio::io_context::executor_type> acceptStrand_;

    ConnectionManager connectionManager_;

    std::uint16_t port_;
    std::size_t countTreads_;
    APIRouter apiRouter_;
    bool enableSignals_;
    std::atomic_bool stopped_{false};
};

} // namespace h1::io_server

#endif
