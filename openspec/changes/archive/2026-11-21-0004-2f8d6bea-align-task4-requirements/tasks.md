# Tasks

## 1. Минимум два потока

- [x] 1.1 В `ServerConfig` (`src/io_server/server_config.cpp`) запретить `threads < 2` для CLI и env, обновить текст ошибки и `--help`; в `IOServer::run()` использовать `std::max<std::size_t>(2, countThreads_)`. Проверка: `ctest --test-dir build -R ServerConfig` зелёный, запуск с `--threads 1` завершается ошибкой
- [x] 1.2 Добавить/обновить тесты `src/test_server_config.cpp`: `--threads 1` и `H1_SERVER_THREADS=1` отклоняются, значения `>=2` принимаются. Проверка: тесты `ServerConfigTest` зелёные
- [x] 1.3 Обновить README: минимум 2 потока, ограничение `--threads`. Проверка: README не содержит примеров с `--threads 1`

## 2. Lifecycle сервера для тестов

- [x] 2.1 В `IOServer` добавить публичные `std::uint16_t port() const` (из `acceptor_.local_endpoint().port()`), `void stop()` (закрыть acceptor, остановить соединения и `io_context`) и параметр конструктора `bool enableSignals = true`. Проверка: `cmd/server_main.cpp` собирается без изменений поведения, `h1Server --help` работает
- [x] 2.2 В тестах использовать `enableSignals=false`, чтобы GTest-процесс не перехватывал `SIGTERM`/`SIGINT`; проверить, что `stop()` корректно завершает `run()` и join-ит поток. Проверка: новый сетевой тест не оставляет висящих потоков и завершается за отведённое время

## 3. Сетевой GTest-клиент

- [x] 3.1 Вынести `MockDBManager` и `MockCalcLocalCache` в `src/test_mocks.hpp`, подключить в `src/test_task_manager_handler.cpp`; поведение существующих тестов не менять. Проверка: `ctest --test-dir build -R TaskManagerHandler` зелёный
- [x] 3.2 Добавить `src/test_http_server.cpp`: фикстура поднимает `IOServer` на `127.0.0.1:0` с `TaskManagerHandler` и моками, busy-циклом с дедлайном ждёт готовности, синхронным Boost.Asio-клиентом отправляет `POST /h1/task`, разбирает статус, заголовки и JSON-тело. Проверка: тест успешного сложения получает 200 и `result=5`
- [x] 3.3 Добавить кейсы: факториал (`value`/`result`), cache hit (`fromCache=true`), деление на ноль (500 + `error`), невалидный JSON (500 + `error`); нигде не использовать `sleep_for`. Проверка: все тесты `HttpServerTest` зелёные, каждый завершается по событию/дедлайну
- [x] 3.4 Зарегистрировать `src/test_http_server.cpp` и `src/test_mocks.hpp`-зависимые файлы в `add_executable(h1_test ...)`; проверить линковку Boost.Asio. Проверка: `cmake --build build --parallel` без ошибок, `ctest -R HttpServer` зелёный

## 4. ThreadSanitizer

- [x] 4.1 Добавить в `CMakeLists.txt` опцию `H1_USE_TSAN` и флаги `-fsanitize=thread -fno-omit-frame-pointer -g` для `h1_core`, `h1Server`, `h1_test`; добавить target `tsan-tests`, запускающий `ctest --output-on-failure`. Проверка: конфигурация с `-DH1_USE_TSAN=ON` и сборка завершаются без ошибок
- [x] 4.2 Добавить в `Makefile` команду `tsan` (конфигурация `build-tsan`, сборка, `ctest`). Проверка: `make tsan` завершается успешно
- [x] 4.3 Прогнать TSAN на итоговой версии: `ctest --test-dir build-tsan --output-on-failure`; при отчётах о гонках/deadlock устранить причины или оформить документированный suppress с обоснованием. Проверка: нет отчётов TSAN, все тесты зелёные; результаты зафиксированы в отчёте. **Примечание:** на текущем ядре TSAN-бинарник требует отключения ASLR, поэтому в TSAN-сборке `ctest` запускает `h1_test` через `setarch $(uname -m) -R`; `make tsan` проходит, 127 тестов, 0 предупреждений

## 5. Документация

- [x] 5.1 Обновить `README.md`: минимум 2 потока, сетевые тесты, TSAN, команда `make tsan`. Проверка: команды из README выполняются
- [x] 5.2 Зафиксировать в спеке `service-quality` и README, что документация ведётся только через OpenSpec, а требование ДЗ-4 о четырёх отдельных документах не применяется по решению разработчика. Проверка: `openspec validate --strict` зелёный, файлы `docs/task4-plan-*.md` отсутствуют

## 6. Ручные кейсы и отчёт

- [x] 6.1 Создать `openspec/changes/0004-2f8d6bea-align-task4-requirements/manual-tests.md`: пререквизиты, минимум 5 кейсов (минимум потоков, сетевой клиент, TSAN, systemd, deb, откат), чеклист `- [ ]`. Проверка: файл существует и самодостаточен
- [x] 6.2 Составить полный отчёт: что сделано, список файлов, тесты, команды самостоятельной проверки, вне-scope проблемы. Проверка: отчёт передан пользователю
- [x] 6.3 Передать пользователю минимум 5 кейсов из `manual-tests.md` и получить подтверждение прогона. Проверка: пользователь явно подтвердил; без этого архивация запрещена

## Workflow follow-up

- Архивировать change после подтверждения ручных кейсов.
- Синхронизировать дельту `service-quality` с основной спекой при архивации.
