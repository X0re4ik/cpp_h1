#pragma once
#ifndef H_H1_IO_SERVER_ROUTER_74e22633a4eda55e0dd7148969ddbed3
#define H_H1_IO_SERVER_ROUTER_74e22633a4eda55e0dd7148969ddbed3

#include "h1/common.hpp"
#include "h1/io_server/handler.hpp"
#include "h1/io_server/request.hpp"
#include "h1/io_server/response.hpp"

#include <cstddef>
#include <memory>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

namespace h1::io_server
{

/// Ключ маршрута: HTTP-метод + нормализованный путь.
struct RouteKey
{
    String method;
    String path;

    bool operator==(const RouteKey&) const = default;
};

struct RouteKeyHash
{
    std::size_t operator()(const RouteKey& key) const;
};

/// Результат поиска маршрута.
enum class RouteMatch
{
    Found,
    MethodNotAllowed, ///< путь зарегистрирован, но с другим методом
    NotFound
};

/// Результат разрешения запроса в роутере.
struct RouteResult
{
    HandlerPtr handler;
    RouteMatch match = RouteMatch::NotFound;
};

/// Роутер HTTP-запросов. Хендлеры регистрируются до запуска сервера,
/// после запуска роутер только читается и потокобезопасен для чтения.
class APIRouter
{
  public:
    /// basePath — префикс, отрезаемый от URI запроса (например, "/h1").
    explicit APIRouter(String basePath);

    /// Зарегистрировать хендлер. Путь нормализуется: query string и
    /// завершающий '/' отбрасываются, basePath (если есть) отрезается.
    void registerHandler(const String& method, const String& path,
                         HandlerPtr handler);

    /// Найти хендлер по методу и URI запроса.
    H1_NODISCARD RouteResult getHandler(const String& method,
                                        const String& uri) const;

  private:
    /// Нормализовать URI: отрезать query string, basePath и завершающий '/'.
    H1_NODISCARD String normalizePath(std::string_view uri) const;

    std::unordered_map<RouteKey, HandlerPtr, RouteKeyHash> router_;
    std::unordered_set<String> registeredPaths_;
    String basePath_;
};

} // namespace h1::io_server

#endif
