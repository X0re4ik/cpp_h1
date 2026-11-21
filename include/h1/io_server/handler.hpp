#pragma once
#ifndef H_H1_IO_SERVER_HANDLER_320b4acdc19001e955a8815b43e23d23
#define H_H1_IO_SERVER_HANDLER_320b4acdc19001e955a8815b43e23d23

#include "h1/common.hpp"
#include "h1/io_server/request.hpp"
#include "h1/io_server/response.hpp"

#include <boost/noncopyable.hpp>

#include <memory>

namespace h1::io_server
{

/// Базовый интерфейс обработчика HTTP-запроса.
/// Конкретные хендлеры наследуются от этого класса и реализуют handleRequest.
class Handler : private boost::noncopyable
{
  public:
    virtual ~Handler() = default;

    virtual void handleRequest(const Request& request,
                               Response& response) const = 0;
};

using HandlerPtr = std::shared_ptr<Handler>;

} // namespace h1::io_server

#endif
