#include "h1/db/manager.hpp"

#include "h1/db/pg/pg_connector.hpp"
#include "h1/db/pg/pg_params.hpp"

#include <iostream>
#include <memory>

namespace h1::db
{

DBManager::DBManager(pg::PGConnectorPtr pgConnect) :
    pgConnect_(std::move(pgConnect))
{}

void DBManager::initSchema()
{
    pgConnect_->executeOrError("CREATE SCHEMA IF NOT EXISTS h1;");
}

void DBManager::createTable()
{

    String sql = R"(
        CREATE TABLE IF NOT EXISTS h1.calculation
        (
            id        BIGINT  GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
            "left"    FLOAT8 NOT NULL,
            "right"   FLOAT8 NOT NULL,
            result    FLOAT8,
            operation TEXT    NOT NULL,
            status    INTEGER NOT NULL,
            error     TEXT,
            is_success BOOLEAN NOT NULL
        );
    )";

    pgConnect_->executeOrError(sql);
}

void DBManager::registerOk(Value_t left, Value_t right, Value_t result,
                           const String& operation, int resultStatus)
{
    registerNewOperation(left, right, result, operation, resultStatus,
                         std::nullopt, true);
}

void DBManager::registerError(Value_t left, Value_t right,
                              const String& operation, int resultStatus,
                              const String& error)
{
    registerNewOperation(left, right, std::nullopt, operation, resultStatus,
                         error, false);
}

std::vector<h1::entity::CalculationResultEntity>
    DBManager::getData(int lastCount)
{
    String sql = R"(
        SELECT
            "id",
            "left",
            "right",
            "result",
            "operation",
            "status",
            "error",
            "is_success"
        FROM
            h1.calculation
        LIMIT $1
    )";

    auto params = pg::PGParams();
    params.add(lastCount);
    params.finalize();

    auto res = pgConnect_->executeParamsOrError(sql, params);

    const auto rows = res.rowsCount();

    using h1::entity::CalculationResultEntity;
    std::vector<CalculationResultEntity> results;
    results.reserve(static_cast<std::size_t>(lastCount) * 2);

    for (int i = 0; i < rows; ++i)
    {
        auto idRaw = res.get<std::int64_t>("id", i);
        auto leftRaw = res.get<CalculationResultEntity::Value_t>("left", i);
        auto rightRaw = res.get<CalculationResultEntity::Value_t>("right", i);
        auto resultRaw = res.get<CalculationResultEntity::Value_t>("result", i);
        auto operationRaw = res.get<String>("operation", i);
        auto statusCodeRaw = res.get<int>("status", i);
        auto errorMessageRaw = res.get<String>("error", i);
        auto isSuccess = res.get<bool>("is_success", i);

        results.emplace_back(
            CalculationResultEntity{.id = *idRaw,
                                    .left = *leftRaw,
                                    .right = *rightRaw,
                                    .result = resultRaw,
                                    .operation = *operationRaw,
                                    .statusCode = *statusCodeRaw,
                                    .errorMessage = errorMessageRaw,
                                    .isSuccess = *isSuccess});
    }

    return results;
}

void DBManager::registerNewOperation(Value_t left, Value_t right,
                                     std::optional<Value_t> result,
                                     const String& operation, int resultStatus,
                                     std::optional<String> error,
                                     bool isSuccess)
{
    String sql = R"(
        INSERT INTO
        h1.calculation("left", "right", "result", "operation", "status", "error", "is_success")
        VALUES
        ($1, $2, $3, $4, $5, $6, $7);
    )";

    auto params = pg::PGParams();
    params.add(left);  // 1
    params.add(right); // 2
    if (!result.has_value())
    {
        params.addNull();
    }
    else
    {
        params.add(*result);
    }                         // 3
    params.add(operation);    // 4
    params.add(resultStatus); // 5
    if (!error.has_value())
    {
        params.addNull();
    }
    else
    {
        params.add(*error);
    }                      // 6
    params.add(isSuccess); // 7

    params.finalize();
    auto res = pgConnect_->executeParamsOrError(sql, params);
}

DBManagerPtr initDataBaseManager()
{
    auto connector = h1::db::pg::makePGConnector();
    auto dbManager = std::make_unique<DBManager>(std::move(connector));
    dbManager->initSchema();
    dbManager->createTable();
    return dbManager;
}

} // namespace h1::db
