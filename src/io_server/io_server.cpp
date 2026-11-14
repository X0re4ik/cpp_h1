#include "h1/io_server/io_server.hpp"

#include "h1/application.hpp"
#include "h1/common.hpp"
#include "h1/db/db.hpp"
#include "h1/io_server/connection.hpp"
#include "h1/io_server/connection_manager.hpp"
#include "h1/io_server/handler.hpp"
#include "h1/io_server/handlers/task_manager.hpp"
#include "h1/io_server/router.hpp"
#include "h1/local_cache/local_cache.hpp"
#include "h1/logger/logger.hpp"

#include <cstddef>
#include <exception>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace h1::io_server
{

void signalHandler(boost::asio::io_context& ioContext,
                   const boost::system::error_code& error, int signalNumber)
{
    if (error)
    {
        // operation_aborted — штатная отмена signal_set, не авария
        if (error == boost::asio::error::operation_aborted)
        {
            return;
        }
        // вот это — реально авария
        H1_LOG_ERROR("Ошибка signal_set: " + error.message());
        return;
    }

    H1_LOG_INFO("Получен сигнал '" + std::to_string(signalNumber) + "'");

    switch (signalNumber)
    {
        case SIGINT:  // 2  — Ctrl+C
        case SIGTERM: // 15 — kill / systemctl stop
            ioContext.stop(); // run() вернётся → main() завершится штатно
            break;

        default:
            // прочие сигналы игнорируем
            break;
    }
}

IOServer::IOServer(const std::string& address, const std::string& port,
                   APIRouter apiRouter, std::size_t countTreads) :
    ioContext_({}),
    //
    signals_(ioContext_),
    //
    acceptor_(ioContext_),
    //
    countTreads_(countTreads),
    //
    apiRouter_(std::move(apiRouter))
{
    using btcp = boost::asio::ip::tcp;
    signals_.add(SIGINT);

    signals_.async_wait(
        [this](const boost::system::error_code& error, int signalNumber) {
            signalHandler(ioContext_, error, signalNumber);
        });

    // Определяем endpoint сервера
    btcp::resolver resolver(ioContext_);
    btcp::resolver::query query(address, port);
    btcp::endpoint endpoint = *resolver.resolve(query);

    // Открываем tcp соединение и привязываем его к endpoint
    acceptor_.open(endpoint.protocol());
    // Разрешаем перезапуск сервера, пока старые сокеты висят в TIME_WAIT
    acceptor_.set_option(btcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen();

    // Запускам процесс обработки событий
    startAccept();
}

void IOServer::run()
{

    std::vector<std::thread> threads;

    for (std::size_t i = 0; i <= countTreads_; ++i)
    {
        threads.emplace_back([this]() { ioContext_.run(); });
    }

    for (auto& thread : threads)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }
}

void IOServer::startAccept()
{
    auto connection =
        makeConnection(ioContext_, connectionManager_, apiRouter_);
    connectionPtr = connection;
    acceptor_.async_accept(connectionPtr->socket(),
                           [this](const boost::system::error_code& exc) {
                               this->handleAccept(exc);
                           });
}

void IOServer::handleAccept(const boost::system::error_code& exc)
{
    if (!acceptor_.is_open())
    {
        return;
    }

    if (!exc)
    {
        connectionManager_.start(connectionPtr);
    }

    startAccept();
}

void IOServer::handleStop()
{
    acceptor_.close();
    connectionManager_.stopAll();
}

} // namespace h1::io_server