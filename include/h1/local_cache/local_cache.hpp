#pragma once
#include <memory>
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

class ICalcLocalCache
{
  public:
    using Result_t = h1::entity::CalculationResultEntity;

    ICalcLocalCache() = default;
    ICalcLocalCache(const ICalcLocalCache&) = delete;
    ICalcLocalCache& operator=(const ICalcLocalCache&) = delete;

    ICalcLocalCache(ICalcLocalCache&&) = default;
    ICalcLocalCache& operator=(ICalcLocalCache&&) = default;
    virtual ~ICalcLocalCache() = default;

    virtual void clear() = 0;

    H1_NODISCARD virtual std::vector<Result_t> values() const = 0;

    // NOTE: Возвращает true, если был создан, false если уже был
    virtual bool add(const OpTask& opTask, Result_t result) = 0;

    H1_NODISCARD virtual Result_t get(const OpTask& opTask) const = 0;

    H1_NODISCARD virtual std::optional<Result_t>
        getIfExist(const OpTask& opTask) const = 0;
};

class CalcLocalCache : public ICalcLocalCache
{
  public:
    using Result_t = h1::entity::CalculationResultEntity;

    void clear() override;

    std::vector<Result_t> values() const override;

    // NOTE: Возвращает true, если был создан, false если уже был
    bool add(const OpTask& opTask, Result_t result) override;

    Result_t get(const OpTask& opTask) const override;

    std::optional<Result_t> getIfExist(const OpTask& opTask) const override;

  private:
    std::unordered_map<OpTask, Result_t, OpTask::HashMethod> local_;
};

using CalcLocalCachePtr = std::unique_ptr<ICalcLocalCache>;
CalcLocalCachePtr initCalcLocalCacheManager();

} // namespace h1::lcache

#endif
