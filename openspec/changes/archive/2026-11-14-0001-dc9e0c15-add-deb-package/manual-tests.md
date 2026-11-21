# Ручные проверки: 0001-dc9e0c15-add-deb-package

Инструкция для человека: прогоните кейсы по порядку, отмечайте чеклист в конце
файла. Все пути от корня репозитория. Архивация change возможна только после
подтвержденного прогона кейсов.

## Пререквизиты

- Ubuntu 22.04 или 24.04 (amd64), доступ в интернет для apt (установка `libpq5`).
- Docker (для PostgreSQL и проверки на 24.04).
- Проект собран: `cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON && cmake --build build --parallel`.
- Переменные БД: `source scripts/set-env.sh` (из `.env`).
- Для кейсов 3 и 5 нужны sudo-правила (установка/удаление пакета).

## Кейс 1 — Сборка пакета с нуля

Проверяет: change воспроизводим, пакет собирается одной командой.

```bash
rm -rf build
make package
ls -la build/h1_*.deb
```

Ожидаемо: сборка без ошибок; файл `build/h1_1.0-1_amd64.deb` существует
(имя содержит имя пакета, версию `1.0-1`, архитектуру `amd64`).

## Кейс 2 — Метаданные и состав пакета

Проверяет: зависимости и состав пакета.

```bash
dpkg -I build/h1_1.0-1_amd64.deb
dpkg -c build/h1_1.0-1_amd64.deb
```

Ожидаемо: `Package: h1`, `Version: 1.0-1`, `Architecture: amd64`; `Depends`
содержит `libpq5` и НЕ содержит spdlog/json/argparse/libenvpp; в составе
только `./usr/bin/h1` (плюс служебные каталоги `usr/`, `usr/bin/`).

## Кейс 3 — Установка и базовый запуск (sudo)

Проверяет: apt разрешает зависимости, бинарник в PATH.

```bash
sudo apt-get install ./build/h1_1.0-1_amd64.deb
which h1
h1 --help
```

Ожидаемо: установка без ошибок (apt при необходимости доустановит `libpq5`);
`which h1` → `/usr/bin/h1`; `h1 --help` печатает справку по аргументам.

## Кейс 4 — Функциональный прогон с БД (успех + ошибка + запись в PostgreSQL)

Проверяет: пакетный бинарник считает, обрабатывает ошибки, пишет историю в БД.

```bash
docker compose -f docker/docker-compose.dev.yaml up -d
source scripts/set-env.sh

h1 --config '{"left": 2, "right": 3, "operation": "+"}'
h1 --config '{"left": 10, "right": 0, "operation": "/"}'

docker exec my-postgres psql -U myuser -d mydb \
  -c 'SELECT "left","right",operation,result,status,is_success FROM h1.calculation ORDER BY id DESC LIMIT 2;'
```

Ожидаемо: `2+3=5.00` в stdout; деление на ноль — сообщение
`Запрещено производить деление на 0` в stderr, stdout пуст; в psql две
последние записи: `2|3|+|5|200|t` и `10|0|/|NULL|500|f`.

## Кейс 5 — Откат: удаление и повторная установка (sudo)

Проверяет: чистое удаление и переустановку той же версии.

```bash
sudo apt-get remove h1
which h1        # пусто
dpkg -l h1      # пакета нет в списке

sudo apt-get install --reinstall ./build/h1_1.0-1_amd64.deb
which h1        # /usr/bin/h1
```

Ожидаемо: после `remove` бинарник удален, пакет не значится;
`--reinstall` той же версии проходит без конфликта; после него `h1` снова в PATH.

## Чеклист прогона

- [x] Кейс 1 — сборка пакета с нуля
- [x] Кейс 2 — метаданные и состав пакета
- [x] Кейс 3 — установка и базовый запуск (sudo)
- [x] Кейс 4 — функциональный прогон с БД
- [x] Кейс 5 — откат: удаление и повторная установка (sudo)

Отметьте кейсы по ходу прогона и сообщите агенту результат — это условие
архивации change (задача 6.3).
