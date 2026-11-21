# Design

## Context

См. `proposal.md` — Why. Change опирается на результат
`0003-7812279c-add-web-server`:

- HTTP-сервер `h1::io_server` на Boost.Asio уже работает, `POST /h1/task`
  возвращает JSON 200/500;
- `IOServer::run()` запускает пул потоков `ioContext_.run()`, `signal_set`
  обрабатывает `SIGINT`/`SIGTERM` на этом же пуле;
- `ServerConfig` разрешает `--threads 1` (проверяется только `== 0`);
- у `IOServer` нет публичного `stop()`/`port()`, поэтому поднять сервер в
  GTest и корректно остановить его сейчас нельзя;
- `loadtest.py` проверяет только HTTP-статусы, а не значение результата;
- TSAN-опции и target в CMake нет.

## Goals / Non-Goals

**Goals:**

- Гарантировать минимум два потока обработки.
- Добавить сетевой GTest-клиент, проверяющий результат расчёта через реальный
  TCP-сокет, без `sleep_for`.
- Добавить TSAN-сборку и прогнать итоговую версию.

**Non-Goals:**

- Выделенный signal-поток: по решению владельца достаточно пула из >=2
  потоков, один из которых принимает сигнал.
- Изменение HTTP-контракта, схемы БД, формата кеша.
- Замена `loadtest.py`; он остаётся нагрузочным smoke-инструментом.
- Изменение состава deb-пакета (уже сделан в 0003).
- Создание четырёх отдельных документов `docs/task4-plan-*.md`: планы ведутся
  в OpenSpec, выбор способа разработки остаётся за разработчиком.

## Decisions

1. **Минимум 2 потока на двух уровнях.**
   `ServerConfig::parse` отклоняет `threads < 2` с `AppException`; в
   `IOServer::run()` используется `std::max<std::size_t>(2, countThreads_)`
   как страховка для прямых вызовов из тестов. Значение по умолчанию остаётся
   4. Альтернатива — отдельный `signalContext_`: отклонена владельцем, так как
   текущая многопоточная схема уже удовлетворяет требованию при >=2 потоках.

2. **Публичный lifecycle у `IOServer`.**
   Добавляются:
   - `std::uint16_t port() const` — реальный порт из
     `acceptor_.local_endpoint().port()` (позволяет биндиться на порт `0`);
   - `void stop()` — через `boost::asio::post(ioContext_, ...)` закрывает
     acceptor, вызывает `handleStop()` и `ioContext_.stop()`.
   Дополнительно конструктор получает `bool enableSignals = true`: тестовый
   сервер отключает `signal_set`, чтобы GTest-процесс не перехватывал
   `SIGTERM`/`SIGINT`.

3. **Сетевой тест — GTest-фикстура, а не отдельный скрипт.**
   Новый `src/test_http_server.cpp`:
   - `MockDBManager` и `MockCalcLocalCache` выносятся в общий
     `src/test_mocks.hpp`, чтобы использовать их и в
     `test_task_manager_handler.cpp`, и в сетевом тесте;
   - фикстура создаёт `APIRouter("/h1")`, регистрирует `TaskManagerHandler`,
     поднимает `IOServer("127.0.0.1", "0", router, 4, false)` в
     `std::thread`, получает фактический порт через `port()`;
   - готовность проверяется busy-циклом: `connect` с `error_code`, при ошибке
     `std::this_thread::yield()`, выход по `steady_clock`-дедлайну (2 секунды);
   - клиент `HttpTestClient` на Boost.Asio синхронно пишет HTTP-запрос,
     читает заголовки через `read_until("\r\n\r\n")`, разбирает
     `Content-Length`, дочитывает тело, возвращает `{status, headers, body}`;
   - тело парсится `nlohmann::json`, проверяются `result`/`error` и
     `fromCache`;
   - `TearDown` вызывает `server.stop()` и `join()` потока.

4. **Тестовые кейсы сетевого клиента.**
   Успешное сложение (200, `result=5`, `fromCache=false`), факториал
   (200, `value`/`result`), cache hit (200, `fromCache=true`), деление на ноль
   (500, непустой `error`), невалидный JSON (500, `error`). Моки
   настраиваются через `EXPECT_CALL`, как в существующем
   `test_task_manager_handler.cpp`.

5. **TSAN — отдельная CMake-опция.**
   `option(H1_USE_TSAN "Build with ThreadSanitizer" OFF)`. При включении к
   `h1_core`, `h1Server` и `h1_test` добавляются
   `-fsanitize=thread -fno-omit-frame-pointer -g` на компиляцию и линковку.
   Target `tsan-tests` запускает `ctest --output-on-failure`. В `Makefile`
   добавляется `make tsan`, который конфигурирует `build-tsan` и прогоняет
   тесты. На ядрах с высоким ASLR-энтропием TSAN-бинарник не стартует без
   отключения рандомизации адресного пространства, поэтому в TSAN-сборке
   `ctest` запускает `h1_test` через `setarch $(uname -m) -R`.
   Альтернатива — ручная команда без CMake-опции: отклонена, так как проверка
   должна быть воспроизводимой.

6. **Документация — только OpenSpec.**
   Планы, требования и решения оформляются в артефактах OpenSpec
   (`proposal.md`, `design.md`, `specs/`, `tasks.md`), а не в четырёх
   отдельных `docs/task4-plan-*.md`. Требование ДЗ-4 о четырёх документах
   намеренно не применяется: способ разработки и формат планов — выбор
   разработчика. README обновляется как пользовательская документация.

## Risks / Trade-offs

- [Сетевой тест занимает локальный порт] → бинд на порт `0` и чтение
  фактического порта через `port()`; при недоступности — диагностический
  `FAIL` по дедлайну.
- [Флейки из-за ожидания сервера] → только busy-цикл с дедлайном и `yield`,
  без фиксированных `sleep_for`.
- [TSAN может дать ложные срабатывания в сторонних библиотеках] → сначала
  прогоняются тесты `h1_test`; при необходимости suppress-файл оформляется
  отдельно и фиксируется в отчёте.
- [TSAN замедляет сборку/тесты] → отдельный `build-tsan`, не влияет на
  обычную сборку.
- [TSAN не стартует из-за ASLR] → в TSAN-сборке тесты запускаются через
  `setarch -R`; ограничение зафиксировано в README и отчёте.
- [Ограничение `threads >= 2` ломает существующие скрипты с `--threads 1`] →
  это осознанное требование ДЗ-4; README и `--help` обновляются.

## Migration Plan

1. Добавить `test_mocks.hpp` и перевести на него существующий
   `test_task_manager_handler.cpp`.
2. Добавить `IOServer::stop()`, `port()`, `enableSignals`; реализовать
   `threads >= 2`.
3. Добавить `src/test_http_server.cpp`, зарегистрировать в CMake.
4. Добавить `H1_USE_TSAN`, `tsan-tests`, `make tsan`.
5. Обновить README и `manual-tests.md`.
6. Откат: изменения локальны в `io_server`, `server_config`, тестах, CMake,
   Makefile и README; HTTP-контракт и БД не меняются.
