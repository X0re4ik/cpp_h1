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

#include <algorithm>
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

namespace
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
        case SIGINT:          // 2  — Ctrl+C
        case SIGTERM:         // 15 — kill / systemctl stop
            ioContext.stop(); // run() вернётся → main() завершится штатно
            break;

        default:
            // прочие сигналы игнорируем
            break;
    }
}

} // namespace

IOServer::IOServer(const std::string& address, const std::string& port,
                   APIRouter apiRouter, std::size_t countTreads,
                   bool enableSignals) :
    ioContext_({}),
    //
    signals_(ioContext_),
    //
    acceptor_(ioContext_),
    //
    acceptStrand_(ioContext_.get_executor()),
    //
    port_(0),
    //
    countTreads_(countTreads),
    //
    apiRouter_(std::move(apiRouter)),
    //
    enableSignals_(enableSignals)
{
    using btcp = boost::asio::ip::tcp;

    if (enableSignals_)
    {
        signals_.add(SIGINT);
        signals_.add(SIGTERM);

        signals_.async_wait(
            [this](const boost::system::error_code& error, int signalNumber) {
                signalHandler(ioContext_, error, signalNumber);
            });
    }

    // Определяем endpoint сервера
    btcp::resolver resolver(ioContext_);
    const btcp::resolver::query query(address, port);
    const btcp::endpoint endpoint = *resolver.resolve(query);

    // Открываем tcp соединение и привязываем его к endpoint
    acceptor_.open(endpoint.protocol());
    // Разрешаем перезапуск сервера, пока старые сокеты висят в TIME_WAIT
    acceptor_.set_option(btcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen();
    port_ = acceptor_.local_endpoint().port();

    // Запускам процесс обработки событий
    startAccept();
}

void IOServer::run()
{

    std::vector<std::thread> threads;

    const auto threadCount = std::max<std::size_t>(2, countTreads_);
    threads.reserve(threadCount);
    for (std::size_t i = 0; i < threadCount; ++i)
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
    acceptor_.async_accept(
        connection->socket(),
        boost::asio::bind_executor(
            acceptStrand_,
            [this, connection](const boost::system::error_code& exc) {
                this->handleAccept(exc, connection);
            }));
}

void IOServer::handleAccept(const boost::system::error_code& exc,
                            const ConnectionPtr& connection)
{
    if (stopped_.load())
    {
        return;
    }

    if (!exc)
    {
        connectionManager_.start(connection);
    }

    if (exc == boost::asio::error::operation_aborted)
    {
        return;
    }

    startAccept();
}

void IOServer::stop()
{
    stopped_.store(true);
    boost::asio::post(acceptStrand_, [this]() {
        handleStop();
        ioContext_.stop();
    });
}

std::uint16_t IOServer::port() const
{
    return port_;
}

void IOServer::handleStop()
{
    acceptor_.close();
    connectionManager_.stopAll();
}

} // namespace h1::io_server