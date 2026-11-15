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

Собрать пакет (требуется обычная сборка; `make package` сам соберет проект):

```bash
make package
# или вручную:
cmake --build build --parallel
cd build && cpack -G DEB
```

Готовый пакет: `build/h1_1.0-1_amd64.deb`.

Установка через apt (apt сам поставит зависимость `libpq5`):

```bash
sudo apt-get install ./build/h1_1.0-1_amd64.deb
```

После установки бинарник доступен как `h1`:

```bash
h1 --help
h1 --config '{"left": 2, "right": 3, "operation": "+"}'
```

Удаление:

```bash
sudo apt-get remove h1
```

Замечание: повторная установка той же версии требует `sudo apt-get install
--reinstall ./build/h1_1.0-1_amd64.deb` либо bump ревизии пакета
(`CPACK_DEBIAN_PACKAGE_RELEASE` в `CMakeLists.txt`).

В состав пакета входит `/usr/bin/h1` и systemd-юниты из `packaging/systemd/`
(см. ниже). PostgreSQL-сервер не является зависимостью пакета — для работы
с БД поднимите PostgreSQL отдельно (например,
`docker compose -f docker/docker-compose.dev.yaml up -d`) и задайте
переменные окружения из `.env`.

## systemd-юниты в пакете

Чтобы добавить юнит в deb-пакет, положите файл с расширением `.service`,
`.timer` или `.socket` в каталог `packaging/systemd/` — при сборке он
автоматически попадёт в `/usr/lib/systemd/system`
(отключается опцией `-DH1_INSTALL_SYSTEMD_UNITS=OFF`).

В пакете уже есть пример `h1.service` (разовый расчёт, `Type=oneshot`).
При установке пакета юнит автоматически включается в автозагрузку
(`postinst` выполняет `systemctl enable h1`), поэтому расчёт выполняется
и при каждой загрузке машины:

```bash
systemctl is-enabled h1     # enabled
sudo systemctl start h1     # выполнить операцию из ExecStart вручную
systemctl status h1         # oneshot: после расчёта inactive (dead)
journalctl -u h1            # результат расчёта
sudo systemctl disable h1   # убрать из автозагрузки
```

Приложению нужны переменные окружения для подключения к PostgreSQL
(`DB_PGHOST`, `DB_PGPORT`, `DB_PGDATABASE`, `DB_PGUSER`, `DB_PGPASSWORD`).
Конфиг ставится пакетом в `/etc/h1/h1.env` (он же читается юнитом) и
зарегистрирован как conffile — dpkg сохранит ваши правки при обновлении:

```bash
sudoedit /etc/h1/h1.env       # укажите свои параметры подключения
```

При установке и удалении пакета скрипты `packaging/deb/postinst` (включение
юнита в автозагрузку), `packaging/deb/prerm` (снятие с автозагрузки при
удалении) и `packaging/deb/postrm` выполняют `systemctl daemon-reload`,
поэтому после добавления нового юнита в `packaging/systemd/` и переустановки
пакета он сразу виден systemd без ручной перезагрузки демона.

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
