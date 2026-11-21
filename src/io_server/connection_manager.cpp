#include "h1/io_server/connection_manager.hpp"

#include <set>
#include <utility>

namespace h1::io_server
{

void ConnectionManager::start(const ConnectionPtr& ptr)
{
    {
        const std::scoped_lock lock(mutex_);
        connections_.insert(ptr);
    }
    ptr->start();
}

void ConnectionManager::stop(const ConnectionPtr& ptr)
{
    {
        const std::scoped_lock lock(mutex_);
        connections_.erase(ptr);
    }
    ptr->stop();
}

void ConnectionManager::stopAll()
{
    std::set<ConnectionPtr> connections;
    {
        const std::scoped_lock lock(mutex_);
        connections.swap(connections_);
    }

    for (const auto& connection : connections)
    {
        connection->stop();
    }
}

} // namespace h1::io_server
