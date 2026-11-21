# h1

`h1` - консольный калькулятор на C++20. Приложение принимает математическую
операцию в виде JSON-строки через аргументы командной строки, выполняет
вычисление и печатает результат.

Проект выполнен в рамках ДЗ-2: основной код организован в ООП-стиле, JSON
парсится через `nlohmann/json`, логирование выполнено через проектную обертку
над `spdlog`, а поведение покрыто unit-тестами на Google Test.

## Возможности

Поддерживаемые операции:

- сложение: `+`
- вычитание: `-`
- умножение: `*`
- деление: `/`
- возведение в степень: `^`
- факториал: `!`

Ошибки вычислений обрабатываются через математическую библиотеку из ДЗ-1:
деление на ноль, отрицательный факториал, отрицательная степень, переполнение и
другие невалидные состояния.

## Зависимости

Для сборки нужны:

- CMake 3.22+
- компилятор с поддержкой C++20
- доступ к GitHub для загрузки зависимостей через `FetchContent`

Проект загружает внешние библиотеки через CMake:

- `spdlog`
- `nlohmann/json`
- `argparse`
- `GoogleTest`
- `cpp_h1_lib`

## Сборка

```bash
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build
```

После сборки приложение находится здесь:

```bash
./build/bin/h1
```

## Сборка deb-пакета

Пакет содержит только серверный режим: бинарник `h1Server`, systemd-юнит
`h1-server.service` и конфиг `/etc/h1/h1-server.env`. CLI-приложение `h1`
собирается для разработки, но в пакет не входит.

Собрать пакет (требуется обычная сборка; `make package` сам соберет проект):

```bash
make package
# или вручную:
cmake --build build --parallel
cd build && cpack -G DEB
```

Готовый пакет: `build/h1-server_1.0-1_amd64.deb`.

Установка через apt (apt сам поставит зависимость `libpq5`):

```bash
sudo apt-get install ./build/h1-server_1.0-1_amd64.deb
```

После установки серверный бинарник доступен как `h1Server`:

```bash
h1Server --help
```

Удаление:

```bash
sudo apt-get remove h1-server
```

Замечание: повторная установка той же версии требует `sudo apt-get install
--reinstall ./build/h1-server_1.0-1_amd64.deb` либо bump ревизии пакета
(`CPACK_DEBIAN_PACKAGE_RELEASE` в `CMakeLists.txt`).

PostgreSQL-сервер не является зависимостью пакета — для работы с БД
поднимите PostgreSQL отдельно (например,
`docker compose -f docker/docker-compose.dev.yaml up -d`) и задайте
переменные окружения из `.env`.

## systemd-юниты в пакете

Чтобы добавить юнит в deb-пакет, положите файл с расширением `.service`,
`.timer` или `.socket` в каталог `packaging/systemd/` — при сборке он
автоматически попадёт в `/usr/lib/systemd/system`
(отключается опцией `-DH1_INSTALL_SYSTEMD_UNITS=OFF`).

В пакет входит `h1-server.service` (`Type=simple`) — долгоживущий
HTTP-сервер `h1Server`. Он читает `DB_*` и `H1_SERVER_*` из
`/etc/h1/h1-server.env` (conffile) и включается в автозагрузку при
установке пакета:

```bash
sudoedit /etc/h1/h1-server.env   # параметры БД и адрес/порт сервера
sudo systemctl start h1-server
systemctl status h1-server       # active (running)
journalctl -u h1-server -f       # логи сервера
sudo systemctl stop h1-server
sudo systemctl disable h1-server # убрать из автозагрузки
```

При установке и удалении пакета скрипты `packaging/deb/postinst` (включение
юнита в автозагрузку), `packaging/deb/prerm` (снятие с автозагрузки и
остановка сервера при удалении) и `packaging/deb/postrm` выполняют
`systemctl daemon-reload`, поэтому после добавления нового юнита в
`packaging/systemd/` и переустановки пакета он сразу виден systemd без ручной
перезагрузки демона.

## Запуск

Приложение принимает JSON через аргумент `--config`.
Флаг `--verbose` включает подробный лог работы программы. Без этого флага
логирование отключено.

Формат для бинарных операций:

```json
{"left": 2, "right": 3, "operation": "+"}
```

Формат для факториала:

```json
{"value": 5, "operation": "!"}
```

Справка:

```bash
./build/bin/h1 --help
```

## Примеры

Сложение:

```bash
./build/bin/h1 --config '{"left": 2, "right": 3, "operation": "+"}'
```

```text
2+3=5.00
```

Деление:

```bash
./build/bin/h1 --config '{"left": 10, "right": 5, "operation": "/"}'
```

```text
10/5=2.00
```

Факториал:

```bash
./build/bin/h1 --config '{"value": 5, "operation": "!"}'
```

```text
5!=120.00
```

Возведение в степень:

```bash
./build/bin/h1 --config '{"left": 2, "right": 8, "operation": "^"}'
```

```text
2^8=256.00
```

Запуск с подробным логом:

```bash
./build/bin/h1 --verbose --config '{"left": 2, "right": 3, "operation": "+"}'
```

В этом режиме приложение дополнительно печатает сообщения логгера о ходе
обработки аргументов, выборе операции и расчете результата.

Пример ошибки:

```bash
./build/bin/h1 --config '{"left": 10, "right": 0, "operation": "/"}'
```

В этом случае приложение печатает сообщение об ошибке в `stderr` и записывает
ошибку в лог.

## Веб-сервер

Помимо CLI-приложения собирается серверный бинарник `h1Server`. Он
предоставляет единственную ручку `POST /h1/task`, которая принимает JSON того
же формата, что и `--config`, и возвращает результат в JSON.

Запуск (нужны переменные `DB_PG*` из `.env`):

```bash
set -a; . ./.env; set +a
./build/bin/h1Server --host 127.0.0.1 --port 8080 --threads 8
```

Параметры можно задать через CLI или окружение (приоритет: CLI → env →
значение по умолчанию):

| CLI | Переменная окружения | По умолчанию |
|---|---|---|
| `--host` | `H1_SERVER_HOST` | `0.0.0.0` |
| `--port` | `H1_SERVER_PORT` | `8080` |
| `--threads` | `H1_SERVER_THREADS` | `4` (минимум `2`) |
| `--verbose` | — | выключено |

`h1Server --help` выводит справку и не требует подключения к PostgreSQL.
Сервер требует минимум 2 потока обработки: значения `--threads 1` и
`H1_SERVER_THREADS=1` отклоняются.

Успешный расчёт — HTTP 200:

```bash
curl -X POST http://localhost:8080/h1/task \
  -H 'Content-Type: application/json' \
  -d '{"left": 2, "right": 3, "operation": "+"}'
```

```json
{"operation":"+","left":2.0,"right":3.0,"result":5.0,"fromCache":false}
```

Факториал:

```bash
curl -X POST http://localhost:8080/h1/task \
  -H 'Content-Type: application/json' \
  -d '{"value": 5, "operation": "!"}'
```

```json
{"operation":"!","value":5.0,"result":120.0,"fromCache":false}
```

Любая ошибка (невалидный JSON, деление на ноль, неизвестная операция) —
HTTP 500 и JSON с текстом ошибки:

```bash
curl -i -X POST http://localhost:8080/h1/task \
  -H 'Content-Type: application/json' \
  -d 'not-json'
```

```json
{"error":"[json.exception.parse_error.101] ..."}
```

Нагрузочная проверка:

```bash
python3 loadtest.py --requests 5000 --concurrency 100
```

## Тесты

Сборка с тестами:

```bash
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build
```

Запуск через CTest:

```bash
ctest --test-dir build --output-on-failure
```

Запуск тестового бинарника напрямую:

```bash
./build/bin/h1_test
```

Сетевой тест `HttpServerTest` поднимает реальный `IOServer` на локальном
порту и проверяет HTTP-статус и JSON-результат через Boost.Asio-клиент:

```bash
ctest --test-dir build -R HttpServer --output-on-failure
```

## ThreadSanitizer

Проверка гонок данных и deadlock:

```bash
cmake -S . -B build-tsan -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON -DH1_USE_TSAN=ON
cmake --build build-tsan --parallel
ctest --test-dir build-tsan --output-on-failure
# или:
make tsan
```

## Документация изменений

Планы, требования и решения ведутся в OpenSpec: `openspec/changes/<change>/`
(proposal, specs, design, tasks). Отдельные документы
`docs/task4-plan-*.md` не используются: формат планов — выбор разработчика.

## Форматирование

Конфигурация с target для форматирования:

```bash
cmake -S . -B build -DUSE_CLANG_FORMAT_FIX=ON
```

Запуск форматирования:

```bash
cmake --build build --target clang-format-fix
```

## Clang-Tidy

Конфигурация с target для статического анализа:

```bash
cmake -S . -B build -DUSE_CLANG_TIDY=ON -DBUILD_TESTING=ON
```

Запуск проверки:

```bash
cmake --build build --target clang-tidy-check
```

## Архитектура

Проект разделен на несколько модулей:

- `cmd/main.cpp` - точка входа, инициализация логгера и запуск приложения.
- `Application` - основной сценарий работы программы: аргументы командной
  строки, настройка логирования, JSON, вычисление, вывод результата и обработка
  ошибок.
- `argparse` - обертка над библиотекой `argparse`, отвечает за CLI и `--help`.
- `json_parser` - преобразует JSON-строку в структуру математической операции.
- `calculator` - ООП-обертка над математической библиотекой из ДЗ-1; для каждой
  операции используется отдельный класс калькулятора.
- `logger` - проектный интерфейс логирования, скрывающий детали `spdlog`.
  Подробный вывод включается CLI-флагом `--verbose`.

Основной поток выполнения:

```text
main
  -> Application::run
      -> H1ArgParse::parse
      -> настройка логгера по --verbose
      -> CalculatorParseJson::parse
      -> CalculatorFactory::makeCalculator
      -> ISimpleCalculator::calculate
      -> вывод результата или ошибки
```

## Структура проекта

```text
cmd/        точка входа приложения
include/    публичные заголовки проекта
src/        реализации модулей и unit-тесты
docs/       дополнительные материалы по заданию и инструментам
```
