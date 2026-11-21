# Ручные проверочные кейсы: 0004-2f8d6bea-align-task4-requirements

Самодостаточная инструкция для человека. Проверяет соответствие ДЗ-4:
минимум потоков, сетевой тестовый клиент, ThreadSanitizer, systemd и deb.

## Пререквизиты

- Проект собран: `cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON && cmake --build build --parallel`.
- Установлены `clang++`, `cmake`, `setarch`, `python3`.
- PostgreSQL поднят, переменные `DB_*` заданы:

```bash
docker compose -f docker/docker-compose.dev.yaml up -d
set -a; . ./.env; set +a
pg_isready -h "$DB_PGHOST" -p "$DB_PGPORT" -U "$DB_PGUSER"
```

- Для кейсов 4–5 нужны `sudo` и systemd.

---

## Кейс 1. Минимум два потока

Проверяет: конфигурация отклоняет меньше двух потоков и принимает два.

```bash
./build/bin/h1Server --threads 1
echo "exit=$?"
H1_SERVER_THREADS=1 ./build/bin/h1Server
echo "exit=$?"
./build/bin/h1Server --help | grep -- --threads
```

**Ожидаемый результат:** первые две команды завершаются с ненулевым кодом и
сообщением «допустимо 2..256»; `--help` показывает «минимум 2».

## Кейс 2. Сетевой GTest-клиент

Проверяет: тест поднимает реальный сервер, отправляет HTTP-запрос и сверяет
JSON-результат.

```bash
ctest --test-dir build -R HttpServer --output-on-failure
```

**Ожидаемый результат:** 5 тестов `HttpServerTest` зелёные: сложение,
факториал, cache hit, деление на ноль, невалидный JSON.

## Кейс 3. ThreadSanitizer

Проверяет: итоговая сборка и тесты проходят без гонок и deadlock.

```bash
make tsan
```

**Ожидаемый результат:** сборка `build-tsan` успешна, `ctest` зелёный, в
выводе нет `WARNING: ThreadSanitizer` и `data race`.

Примечание: на ядрах с высоким ASLR-энтропием TSAN-бинарник запускается через
`setarch $(uname -m) -R`; это уже учтено в CMake для `ctest`.

## Кейс 4. systemd: start / restart / stop

Проверяет: сервис управляется через systemd.

```bash
make package
sudo apt-get install -y ./build/h1-server_1.0-1_amd64.deb
sudoedit /etc/h1/h1-server.env   # укажите реальные DB_* и H1_SERVER_PORT
sudo systemctl start h1-server
systemctl is-active h1-server
sudo systemctl restart h1-server
systemctl is-active h1-server
curl -sS -X POST http://127.0.0.1:8080/h1/task \
  -H 'Content-Type: application/json' \
  -d '{"left": 2, "right": 3, "operation": "+"}'
sudo systemctl stop h1-server
systemctl is-active h1-server
```

**Ожидаемый результат:** после `start` и `restart` — `active`; `curl`
возвращает JSON с `result:5`; после `stop` — `inactive`.

## Кейс 5. deb-пакет содержит только сервер

Проверяет: состав пакета и conffiles.

```bash
dpkg -c build/h1-server_1.0-1_amd64.deb
rm -rf /tmp/h1-ctrl && dpkg -e build/h1-server_1.0-1_amd64.deb /tmp/h1-ctrl
cat /tmp/h1-ctrl/conffiles
sudo apt-get remove -y h1-server
```

**Ожидаемый результат:** в пакете есть `/usr/bin/h1Server`,
`/usr/lib/systemd/system/h1-server.service`,
`/etc/h1/h1-server.env`; нет `/usr/bin/h1`, `h1.service`, `h1.env`; в
`conffiles` только `/etc/h1/h1-server.env`.

## Кейс 6. Нагрузка после изменений

Проверяет: сервер по-прежнему не падает и не блокируется.

```bash
set -a; . ./.env; set +a
./build/bin/h1Server --host 127.0.0.1 --port 8080 --threads 8 &
SERVER_PID=$!
sleep 1
python3 loadtest.py --requests 5000 --concurrency 100
curl -sS -o /dev/null -w '%{http_code}\n' -X POST http://127.0.0.1:8080/h1/task \
  -H 'Content-Type: application/json' \
  -d '{"left": 7, "right": 6, "operation": "*"}'
kill "$SERVER_PID"
```

**Ожидаемый результат:** `Errors: 0`, `Status codes: {200: 5000}`,
контрольный `curl` — `200`; сервер не падает.

## Кейс 7. Откат

Проверяет: окружение возвращается в исходное состояние.

```bash
kill "$SERVER_PID" 2>/dev/null || true
docker compose -f docker/docker-compose.dev.yaml down
git status --short
```

**Ожидаемый результат:** сервер и контейнер остановлены, `git status` показывает
только ожидаемые файлы change.

---

## Чеклист прогона

- [+] Кейс 1. Минимум два потока
- [+] Кейс 2. Сетевой GTest-клиент
- [+] Кейс 3. ThreadSanitizer
- [+] Кейс 4. systemd: start / restart / stop
- [+] Кейс 5. deb-пакет содержит только сервер
- [+] Кейс 6. Нагрузка после изменений
- [+] Кейс 7. Откат
