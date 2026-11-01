#include "h1/io_server/connection_manager.hpp"

#include <algorithm>

namespace h1::io_server
{

void ConnectionManager::start(ConnectionPtr ptr)
{
    connections_.insert(ptr);
    ptr->start();
}

void ConnectionManager::stop(ConnectionPtr ptr)
{
    connections_.erase(ptr);
    ptr->stop();
}

void ConnectionManager::stopAll()
{
    // Останавливаем все соединения
    std::for_each(std::begin(connections_), std::end(connections_),
                  [](ConnectionPtr ptr) { ptr->stop(); });

    connections_.clear();
}

} // namespace h1::io_server