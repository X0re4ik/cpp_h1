#include "h1/io_server/router.hpp"

#include "h1/logger/logger.hpp"

#include <utility>

namespace h1::io_server
{

namespace
{

constexpr std::size_t hashSeed = 0x9e3779b97f4a7c15ULL;

std::size_t hashCombine(std::size_t seed, std::size_t value)
{
    return seed ^ (value + hashSeed + (seed << 6) + (seed >> 2));
}

} // namespace

std::size_t RouteKeyHash::operator()(const RouteKey& key) const
{
    return hashCombine(std::hash<String>{}(key.method),
                       std::hash<String>{}(key.path));
}

String APIRouter::normalizePath(std::string_view uri) const
{
    // Отрезать query string и fragment
    const auto queryPos = uri.find_first_of("?#");
    if (queryPos != std::string_view::npos)
    {
        uri.remove_suffix(uri.size() - queryPos);
    }

    // Отрезать basePath
    if (!basePath_.empty() && uri.starts_with(basePath_))
    {
        uri.remove_prefix(basePath_.size());
    }

    String path(uri);

    // Гарантировать ведущий '/'
    if (path.empty() || path.front() != '/')
    {
        path.insert(path.begin(), '/');
    }

    // Схлопывать завершающий '/' ("/test/" -> "/test"), кроме корня
    if (path.size() > 1 && path.back() == '/')
    {
        path.pop_back();
    }

    return path;
}

APIRouter::APIRouter(String basePath) : basePath_(std::move(basePath))
{}

void APIRouter::registerHandler(const String& method, const String& path,
                                HandlerPtr handler)
{
    const RouteKey key{method, normalizePath(path)};

    if (!router_.emplace(key, std::move(handler)).second)
    {
        H1_LOG_WARN("Маршрут '" + key.method + " " + key.path +
                    "' уже зарегистрирован и будет перезаписан");
        router_[key] = std::move(handler);
    }
    registeredPaths_.insert(key.path);
}

RouteResult APIRouter::getHandler(const String& method, const String& uri) const
{
    const String path = normalizePath(uri);

    RouteResult result;
    if (auto iter = router_.find(RouteKey{method, path});
        iter != std::end(router_))
    {
        result.handler = iter->second;
        result.match = RouteMatch::Found;
        return result;
    }

    if (registeredPaths_.find(path) != std::end(registeredPaths_))
    {
        result.match = RouteMatch::MethodNotAllowed;
        return result;
    }

    result.match = RouteMatch::NotFound;
    return result;
}

} // namespace h1::io_server
