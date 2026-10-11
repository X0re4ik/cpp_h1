#pragma once
#ifndef H_H1_DB_MANAGER_4dedd84d589a84161091ef1baee307d7
#define H_H1_DB_MANAGER_4dedd84d589a84161091ef1baee307d7

#include "./pg/pg_connector.hpp"
#include "h1/common.hpp"
#include "h1/entities/CalculationResult/model.hpp"

#include <optional>
#include <vector>

namespace h1::db
{

class IDBManager
{

  public:
    using Value_t = h1::entity::CalculationResultEntity::Value_t;

    IDBManager() = default;
    IDBManager(const IDBManager&) = delete;
    IDBManager& operator=(const IDBManager&) = delete;

    IDBManager(IDBManager&&) = default;
    IDBManager& operator=(IDBManager&&) = default;

    virtual ~IDBManager() = default;

    virtual void registerOk(Value_t left, Value_t right, Value_t result,
                            const String& operation, int resultStatus) = 0;

    virtual void registerError(Value_t left, Value_t right,
                               const String& operation, int resultStatus,
                               const String& error) = 0;

    virtual std::vector<h1::entity::CalculationResultEntity>
        getData(int lastCount) = 0;
};

class DBManager : public IDBManager
{
  public:
    explicit DBManager(pg::PGConnectorPtr pgConnect);

    void initSchema();

    void createTable();

    void registerOk(Value_t left, Value_t right, Value_t result,
                    const String& operation, int resultStatus) override;

    void registerError(Value_t left, Value_t right, const String& operation,
                       int resultStatus, const String& error) override;

    std::vector<h1::entity::CalculationResultEntity>
        getData(int lastCount) override;

  private:
    void registerNewOperation(Value_t left, Value_t right,
                              std::optional<Value_t> result,
                              const String& operation, int resultStatus,
                              std::optional<String> error, bool isSuccess);

    pg::PGConnectorPtr pgConnect_;
};

using DBManagerPtr = std::unique_ptr<IDBManager>;
DBManagerPtr initDataBaseManager();

} // namespace h1::db

#endif
