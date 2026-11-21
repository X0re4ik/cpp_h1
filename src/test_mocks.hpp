#ifndef H_H1_TEST_MOCKS_8d1c4f5a2b3e6d7c9a0f1e2d3c4b5a69
#define H_H1_TEST_MOCKS_8d1c4f5a2b3e6d7c9a0f1e2d3c4b5a69

#include "h1/db/manager.hpp"
#include "h1/local_cache/local_cache.hpp"

#include <optional>
#include <vector>

#include <gmock/gmock.h>

namespace h1::test
{

class MockDBManager : public db::IDBManager
{
  public:
    MOCK_METHOD(void, registerOk,
                (Value_t, Value_t, Value_t, const h1::String&, int),
                (override));

    MOCK_METHOD(void, registerError,
                (Value_t, Value_t, const h1::String&, int, const h1::String&),
                (override));

    MOCK_METHOD(std::vector<h1::entity::CalculationResultEntity>, getData,
                (int), (override));
};

class MockCalcLocalCache : public lcache::ICalcLocalCache
{
  public:
    MOCK_METHOD(void, clear, (), (override));

    MOCK_METHOD(std::vector<Result_t>, values, (), (const, override));

    MOCK_METHOD(bool, add, (const h1::lcache::OpTask&, Result_t), (override));

    MOCK_METHOD(Result_t, get, (const h1::lcache::OpTask&), (const, override));

    MOCK_METHOD(std::optional<Result_t>, getIfExist,
                (const h1::lcache::OpTask&), (const, override));
};

} // namespace h1::test

#endif
