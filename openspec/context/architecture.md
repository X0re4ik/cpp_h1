# Архитектура проекта h1

Язык: C++20, компилятор clang++, CMake 3.22+. Сборка: статическая библиотека
`h1_core` + исполняемый файл `h1` (точка входа `cmd/main.cpp`).

## Структура каталогов

Все публичные заголовки — в `include/h1/<module>/`, реализации — в `src/`
(зеркально). Тесты — `src/test_*.cpp`.

## Модули (namespace → заголовок → реализация)

- `h1` — `include/h1/application.hpp`, `src/application.cpp` — **Application**:
  оркестрация. Парсинг CLI, настройка логгера по `--verbose`, парсинг JSON,
  расчет, вывод результата/ошибки, прогрев кеша из БД.
- `h1::argp` — `include/h1/argparse/` — обертка над библиотекой `argparse`,
  CLI и `--help`.
- `h1::jsonp` — `include/h1/json_parser/` — JSON → `MathOperation`
  (nlohmann/json). `json_parser.hpp` — агрегатор над `impl.hpp` + `exceptions.hpp`.
- `h1::calc` — `include/h1/calculator/` — интерфейс `ISimpleCalculator`
  (`calculate()`), база `BaseSimpleCalculator` c вложенной структурой `Task`,
  по классу на операцию: `Addition/Subtraction/Multiplication/Division/Factorial/Power`
  Calculator. `di.cpp` — фабрика калькуляторов. Адаптирует внешнюю C-библиотеку
  `cpp_h1_lib` (FetchContent, header-only: `h1.hpp`, `task.hpp`, функции
  `createTask/deleteTask`, `makeAddition` и т.п.).
- `h1::calculation` — `include/h1/calculation/service.hpp` — **CalculationService**:
  cache hit → результат из кеша; miss → расчет через калькулятор, запись в БД
  (`registerOk`/`registerError`) и в кеш.
- `h1::db` — `include/h1/db/manager.hpp` — интерфейс `IDBManager`
  (`registerOk`, `registerError`, `getData`), реализация `DBManager` над
  `h1::db::pg`. Схема `h1`, таблица `h1.calculation`
  (left, right, operation, result, status, error, is_success) создается
  в `src/db/manager.cpp` (`initSchema`/`createTable`).
- `h1::db::pg` — `include/h1/db/pg/` — собственная минимальная RAII-обертка
  над libpq: `PGConnector` (`PGconnPtr` = unique_ptr с deleter'ом),
  `PGRawResult` (PGresult), `PGParams`. `pg.hpp` — агрегатор.
- `h1::lcache` — `include/h1/local_cache/` — интерфейс `ICalcLocalCache`,
  реализация `CalcLocalCache` на `std::unordered_map<OpTask, Result_t, Hash>`;
  ключ `OpTask` (left, right, operation) с кастомным хешем; для `+` и `*`
  порядок операндов нормализуется ("1+2" == "2+1").
- `h1::log` — `include/h1/logger/` — проектный логгер, скрывающий spdlog
  (Singleton Мейерса). Использовать только макросы
  `H1_LOG_INFO/H1_LOG_DEBUG/H1_LOG_WARN/H1_LOG_ERROR` из `logger.hpp`;
  конфигурация через `initLoggerConfig` (LoggerConfig: консольный sink,
  pattern, уровень).
- `h1::entity` — `include/h1/entities/CalculationResult/model.hpp` — общая
  сущность `CalculationResultEntity`.
- `h1::config` — `include/h1/config/` — чтение env-переменных БД
  (`DB_PGHOST`, `DB_PGPORT`, `DB_PGDATABASE`, `DB_PGUSER`, `DB_PGPASSWORD`)
  через libenvpp.

## Поток выполнения

```text
main (cmd/main.cpp)
  -> initDataBaseManager() / initCalcLocalCacheManager()  (фабрики, unique_ptr)
  -> Application::run(argc, argv)
      -> H1ArgParse::parse            (CLI: --config, --verbose, --help)
      -> настройка логгера по --verbose
      -> CalculatorParseJson::parse   (JSON -> MathOperation)
      -> CalculationService::calculate
          -> ICalcLocalCache::getIfExist   (cache hit -> результат)
          -> cache miss: ISimpleCalculator::calculate
             -> запись в БД (registerOk/registerError)
             -> запись в кеш (localCache_.add)
      -> вывод результата или ошибки в stderr
```

## Зависимости

Через FetchContent: `spdlog`, `nlohmann/json`, `argparse`, `libenvpp`,
`GoogleTest`, `cpp_h1_lib` (header-only C-библиотека математики из ДЗ-1).
Через `find_package`: `PostgreSQL` (libpq). **libpqxx запрещен.**

Схема БД создается приложением при старте (`DBManager::initSchema/createTable`).
