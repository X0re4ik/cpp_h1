#pragma once
#ifndef H_H1_LOCAL_CACHE_b7d089624a98ddc87ca90b6d807d9627
#define H_H1_LOCAL_CACHE_b7d089624a98ddc87ca90b6d807d9627

#include "h1/common.hpp"
#include "h1/entities/CalculationResult/model.hpp"

#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace h1::lcache
{

struct OpTask
{
    struct HashMethod
    {
        using Hash_t = std::size_t;
        Hash_t operator()(const OpTask& opTask) const H1_NOEXCEPT;
    };

    using Value_t = double;

    // NOLINTNEXTLINE (misc-non-private-member-variables-in-classes)
    Value_t left;

    // NOLINTNEXTLINE (misc-non-private-member-variables-in-classes)
    Value_t right;

    // NOLINTNEXTLINE (misc-non-private-member-variables-in-classes)
    String operation;

    bool operator==(const OpTask& other) const H1_NOEXCEPT;
};

class CalcLocalCache
{
  public:
    using Result_t = h1::entity::CalculationResultEntity;

    void clear();

    std::vector<Result_t> values() const;

    // NOTE: Возвращает true, если был создан, false если уже был
    bool add(const OpTask& opTask, Result_t result);

    Result_t get(const OpTask& opTask) const;

    std::optional<Result_t> getIfExist(const OpTask& opTask) const;

  private:
    std::unordered_map<OpTask, Result_t, OpTask::HashMethod> local_;
};

} // namespace h1::lcache

#endif
