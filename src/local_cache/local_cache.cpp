#include "h1/local_cache/local_cache.hpp"

#include <utility>

namespace h1::lcache
{

bool OpTask::operator==(const OpTask& other) const H1_NOEXCEPT
{
    return left == other.left && right == other.right &&
           operation == other.operation;
}

OpTask::HashMethod::Hash_t
    OpTask::HashMethod::operator()(const OpTask& opTask) const H1_NOEXCEPT
{
    auto h1 = std::hash<std::string>()(opTask.operation);
    auto h2 = std::hash<OpTask::Value_t>()(opTask.left);
    auto h3 = std::hash<OpTask::Value_t>()(opTask.right);

    Hash_t seed = h1;
    seed ^= h2 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed ^= h3 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    return seed;
}

void CalcLocalCache::clear()
{
    local_.clear();
}

std::vector<CalcLocalCache::Result_t> CalcLocalCache::values() const
{
    std::vector<Result_t> localValues;
    localValues.reserve(local_.size() * 2);
    for (const auto& [_, value] : local_)
    {
        localValues.push_back(value);
    }
    return localValues;
}

// NOTE: Возвращает true, если был создан, false если уже был
bool CalcLocalCache::add(const OpTask& opTask, Result_t result)
{
    auto iter = local_.find(opTask);
    if (iter == local_.end())
    {
        return false;
    }
    local_[opTask] = std::move(result);
    return true;
}

CalcLocalCache::Result_t CalcLocalCache::get(const OpTask& opTask) const
{
    return local_.at(opTask);
}

std::optional<CalcLocalCache::Result_t>
    CalcLocalCache::getIfExist(const OpTask& opTask) const
{
    auto iter = local_.find(opTask);
    if (iter == local_.end())
    {
        return std::nullopt;
    }

    return iter->second;
}

} // namespace h1::lcache
