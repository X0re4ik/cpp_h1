#include "h1/io_server/io_server.hpp"

#include "h1/io_server/connection.hpp"

namespace h1::io_server
{
IOServer::IOServer(const std::string& address, const std::string& port) :
    ioContext_({}), signals_(ioContext_), acceptor_(ioContext_)
{
    using btcp = boost::asio::ip::tcp;
    signals_.add(SIGINT);

    // Определяем endpoint сервера
    btcp::resolver resolver(ioContext_);
    btcp::resolver::query query(address, port);
    btcp::endpoint endpoint = *resolver.resolve(query);

    // Открываем tcp соединение и привязываем его к endpoint
    acceptor_.open(endpoint.protocol());
    acceptor_.bind(endpoint);
    acceptor_.listen();

    // Запускам процесс обработки событий
    startAccept();
}

void IOServer::run()
{
    ioContext_.run();
}

void IOServer::startAccept()
{
    auto connection = makeConnection(ioContext_, connectionManager_);
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