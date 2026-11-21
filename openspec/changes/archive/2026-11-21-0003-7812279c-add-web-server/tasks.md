# Tasks

## 1. Исправление потокобезопасности БД и кеша

- [x] 1.1 Убрать повторный `std::lock_guard` на одном мьютексе в `DBManager::registerNewOperation()` (`src/db/manager.cpp`): оставить ровно один захват на функцию, проверить, что нет самоблокировки. Проверка: сборка `cmake --build build --parallel` без ошибок, существующие тесты `ctest --test-dir build --output-on-failure` зелёные
- [x] 1.2 Проверить остальные методы `DBManager` (`initSchema`, `createTable`, `getData`) и `CalcLocalCache` на отсутствие двойных захватов и гонок; при необходимости привести к единому стилю RAII-блокировок. Проверка: ручной просмотр diff, отсутствие `lock_guard` внутри уже захваченной критической секции; зафиксировать в отчёте, что реальная проверка БД — в ручных кейсах (unit-тесты без PostgreSQL)

## 2. JSON-контракт расчёта и хендлер

- [x] 2.1 Ввести структуру `CalculationReply` и изменить `ISmartCalculatorService::calculate()` на возврат результата вместо `void` (`include/h1/io_server/handlers/task_manager.hpp`, `src/io_server/handlers/task_manager.cpp`): операция, левый/правый операнд, `result`, `errorMessage`, `isSuccess`, `fromCache`. Проверка: модуль компилируется в составе `h1_core`
- [x] 2.2 Реализовать в `TaskManagerHandler` JSON-ответ через `nlohmann/json`: успех — 200 с `operation/left/right/result/fromCache` (для факториала `value` вместо `left/right`), любая ошибка — 500 с `{"error":"..."}`, заголовки `Content-Type: application/json` и `Content-Length`. Проверка: новый unit-тест на успешный ответ проходит
- [x] 2.3 Добавить `src/test_task_manager_handler.cpp` с `MockDBManager` и `MockCalcLocalCache`: успешный расчёт (200, JSON, `fromCache:false`, вызов `registerOk`), попадание в кеш (`fromCache:true`, без `registerOk`), деление на ноль (500, `error`, вызов `registerError`), невалидный JSON (500, `error`, без обращений к БД). Зарегистрировать файл в `add_executable(h1_test ...)`. Проверка: `ctest --test-dir build -R TaskManagerHandler` зелёный

## 3. Конфигурация сервера

- [x] 3.1 Добавить `include/h1/io_server/server_config.hpp` и `src/io_server/server_config.cpp`: `ServerConfig` с полями `host`, `port`, `threads`, `verbose`; разбор CLI (`--host`, `--port`, `--threads`, `--verbose`, `--help`) через `argparse`; переменные `H1_SERVER_HOST`, `H1_SERVER_PORT`, `H1_SERVER_THREADS`; приоритет CLI → env → default (`0.0.0.0`, `8080`, `4`). Проверка: модуль компилируется, `h1Server --help` работает без `DB_PG*`
- [x] 3.2 Добавить `src/test_server_config.cpp`: значения по умолчанию, переопределение через env, приоритет CLI над env, разбор `--verbose`, поведение `--help`. Зарегистрировать файл в `add_executable(h1_test ...)`. Проверка: `ctest --test-dir build -R ServerConfig` зелёный

## 4. Роутер

- [x] 4.1 Добавить `src/test_io_server_router.cpp`: найденный маршрут, `MethodNotAllowed` для зарегистрированного пути с другим методом, `NotFound` для неизвестного пути, нормализация basePath `/h1`, query string и завершающего `/`. Проверка: `ctest --test-dir build -R APIRouter` зелёный

## 5. Запуск сервера, прогрев кеша, логгер

- [x] 5.1 Вынести прогрев кеша из `src/application.cpp` в переиспользуемую функцию (объявление в `include/h1/application.hpp`) и вызывать её из `Application::run` и `cmd/server_main.cpp` до старта сервера. Проверка: CLI-тесты и сборка зелёные, в `server_main.cpp` кеш прогревается до `IOServer::run()`
- [x] 5.2 Исправить `initLogger` в `cmd/server_main.cpp`: применять вычисленный уровень (`DEBUG`/`OFF`), имя логгера и паттерн; убрать неиспользуемую переменную. Проверка: `h1Server --verbose` пишет отладочные сообщения, без флага — не пишет
- [x] 5.3 Перевести `cmd/server_main.cpp` на `ServerConfig`: убрать захардкоженные `0.0.0.0:8080` и число потоков, передавать их в `IOServer`; сохранить регистрацию `POST /h1/task` и базовый путь `/h1`. Проверка: запуск с `--host/--port` поднимает сервер на заданном адресе, `--help` завершается без БД

## 6. Сборка, упаковка и документация

- [x] 6.1 Обновить `CMakeLists.txt`: добавить `src/io_server/server_config.cpp` в `H1_CORE_SOURCES`, новые тестовые файлы в `add_executable(h1_test ...)`, install-правило для `h1Server` в `${CMAKE_INSTALL_BINDIR}`. Проверка: `cmake -S . -B build -DBUILD_TESTING=ON && cmake --build build --parallel` без ошибок
- [x] 6.2 Оставить в упаковке только сервер: удалить `packaging/systemd/h1.service` и `packaging/etc/h1/h1.env`, добавить `packaging/systemd/h1-server.service` (`Type=simple`, `Restart=on-failure`, `EnvironmentFile=-/etc/h1/h1-server.env`, `ExecStart=/usr/bin/h1Server`) и `packaging/etc/h1/h1-server.env` с `DB_*` и `H1_SERVER_*`; в `packaging/deb/conffiles` оставить только `/etc/h1/h1-server.env`, в `postinst/prerm` управлять только `h1-server`. Проверка: `make package`, `dpkg -c build/h1-server_*_amd64.deb` содержит только `/usr/bin/h1Server`, `/usr/lib/systemd/system/h1-server.service`, `/etc/h1/h1-server.env`; `/usr/bin/h1`, `h1.service` и `h1.env` отсутствуют; `dpkg -e` показывает один conffile
- [x] 6.3 Обновить `README.md`: запуск `h1Server`, параметры CLI/env, контракт `POST /h1/task` с примерами `curl` для успеха и ошибки, установка/управление `h1-server.service`. Проверка: команды из README выполняются как написано

## 7. Интеграционная проверка

- [x] 7.1 Отформатировать код: `cmake -S . -B build -DUSE_CLANG_FORMAT_FIX=ON && cmake --build build --target clang-format-fix`; проверить `git diff` на отсутствие лишних переформатирований чужих файлов. Проверка: цель отрабатывает без ошибок
- [x] 7.2 Запустить `clang-tidy`: `cmake -S . -B build -DBUILD_TESTING=ON -DUSE_CLANG_TIDY=ON && cmake --build build --target clang-tidy-check`. Проверка: новые/изменённые файлы без предупреждений; предсуществующие ошибки вне scope зафиксировать в отчёте и не исправлять без согласования. **Примечание:** все файлы change (server_config, connection, connection_manager, task_manager, io_server, server_main, db/manager, новые тесты) — 0 предупреждений. Цель `clang-tidy-check` всё ещё падает на pre-existing файлах вне scope: `src/io_server/response.cpp` (47), `src/io_server/request_parser.cpp` (23), `src/local_cache/local_cache.cpp` (11), `src/test_request_parser.cpp` (6), `src/io_server/router.cpp` (4), `src/calculation/service.cpp` (3), `src/application.cpp` (1), `_deps/fmt/base.h` (1) — не исправлялись
- [x] 7.3 Прогнать все тесты: `ctest --test-dir build --output-on-failure`. Проверка: все тесты зелёные, включая новые
- [x] 7.4 Прогнать Valgrind по тестам: `ctest --test-dir build -T memcheck` (или `cmake --build build --target valgrind-tests`). Проверка: нет утечек и ошибок памяти, `--error-exitcode=42` не срабатывает. **Примечание:** новые тесты (APIRouter, TaskManagerHandler, ServerConfig) прошли Memcheck без дефектов; отчёт содержит 4 `Potential Memory Leak` в pre-existing `TestApplication` (`src/test_application.cpp`) — вне scope
- [x] 7.5 Проверить сервер вручную: поднять PostgreSQL (`docker compose -f docker/docker-compose.dev.yaml up -d`), задать `DB_*`, запустить `./build/bin/h1Server --port 8080`, выполнить `curl` успешного расчёта, ошибочного расчёта и повторного запроса (cache hit). Проверка: коды 200/500, JSON-тело, `fromCache` меняется с `false` на `true`
- [x] 7.6 Прогнать нагрузочный тест `python3 loadtest.py --requests 5000 --concurrency 100` против запущенного сервера. Проверка: `Errors: 0`, все ответы — ожидаемые статусы, сервер не падает и после прогона отвечает на контрольный `curl`; при зависаниях/таймаутах зафиксировать и устранить причину блокировки до завершения задачи

## 8. Ручные кейсы и отчёт

- [x] 8.1 Создать `openspec/changes/0003-7812279c-add-web-server/manual-tests.md`: пререквизиты (PostgreSQL, `.env`, собранный проект), минимум 5 кейсов (успешный расчёт, ошибка вычисления, невалидный JSON, cache hit, конфигурация CLI/env, systemd-юнит, состав deb-пакета, откат/удаление), чеклист `- [ ]`. Проверка: файл существует, самодостаточен, содержит команды и ожидаемый результат
- [x] 8.2 Составить для пользователя полный отчёт о цикле изменений: что сделано по каждой задаче, полный список созданных/изменённых файлов с кратким описанием, добавленные тесты, команды самостоятельной проверки, зафиксированные вне-scope проблемы. Проверка: отчёт передан пользователю в ответе агента
- [x] 8.3 Передать пользователю минимум 5 ручных проверочных кейсов из `manual-tests.md` и получить подтверждение их прогона. Проверка: пользователь явно подтвердил прогон кейсов; без этого архивация change запрещена

## Workflow follow-up

- Архивировать change после подтверждения прогона ручных кейсов и выполнения требований проекта к ревью.
- Перед архивацией синхронизировать дельты `http-api` и `deb-packaging` с основными спеками.
