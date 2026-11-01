#ifndef H_H1_IO_SERVER_CONNECTION_MANAGER_ff0ffc277c22f717cb8d9b7b44e01c41
#define H_H1_IO_SERVER_CONNECTION_MANAGER_ff0ffc277c22f717cb8d9b7b44e01c41

#include "h1/io_server/connection.hpp"

#include <boost/noncopyable.hpp>

#include <set>

namespace h1::io_server
{

class ConnectionManager : private boost::noncopyable
{
  public:
    void start(ConnectionPtr);

    void stop(ConnectionPtr);

    /// Stop all connections.
    void stopAll();

  private:
    /// The managed connections.
    std::set<ConnectionPtr> connections_;
};

} // namespace h1::io_server

#endif // HTTP_CONNECTION_MANAGER_HPP
