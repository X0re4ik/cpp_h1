#pragma once
#ifndef H_H1_IO_SERVER_HANDLER_TASK_MANAGER_070b2a5b0882b99206b566e11dd40c42
#define H_H1_IO_SERVER_HANDLER_TASK_MANAGER_070b2a5b0882b99206b566e11dd40c42

#include "h1/application.hpp"
#include "h1/common.hpp"
#include "h1/db/db.hpp"
#include "h1/db/manager.hpp"
#include "h1/exceptions.hpp"
#include "h1/io_server/handler.hpp"
#include "h1/json_parser/impl.hpp"
#include "h1/json_parser/json_parser.hpp"
#include "h1/local_cache/local_cache.hpp"
#include "h1/logger/logger.hpp"

#include <memory>

namespace h1::io_server
{

class ISmartCalculatorService
{
  public:
    virtual void calculate(const String& jsonData) = 0;

    virtual ~ISmartCalculatorService() = default;
};

using SmartCalculatorServicePtr = std::unique_ptr<ISmartCalculatorService>;

/// Базовый интерфейс обработчика HTTP-запроса.
/// Конкретные хендлеры наследуются от этого класса и реализуют handleRequest.
class TaskManagerHandler : public Handler
{
  public:
    explicit TaskManagerHandler(SmartCalculatorServicePtr);

    void handleRequest(const Request& request,
                       Response& response) const override;

  private:
    SmartCalculatorServicePtr smartCalculatorServicePtr_;
};

HandlerPtr makeTaskManagerHandler(h1::db::IDBManager& dbManager,
                                  h1::lcache::ICalcLocalCache& localCache);

} // namespace h1::io_server

#endif
