# Ручные проверочные кейсы: 0003-7812279c-add-web-server

Самодостаточная инструкция для человека. Прогон кейсов подтверждает, что
серверный режим работает, возвращает JSON, сохраняет результаты в PostgreSQL,
не блокируется и не падает под конкурентной нагрузкой.

## Пререквизиты

- Проект собран: `cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON && cmake --build build --parallel`.
- Docker и `docker compose` доступны; порт `5432` свободен.
- Для кейсов 7–8 нужны `sudo` и systemd (обычный Ubuntu, не контейнер).
- Терминал из корня репозитория.

Поднять PostgreSQL и подготовить окружение:

```bash
docker compose -f docker/docker-compose.dev.yaml up -d
set -a; . ./.env; set +a
pg_isready -h "$DB_PGHOST" -p "$DB_PGPORT" -U "$DB_PGUSER"
```

Ожидаемо: `accepting connections`.

---

## Кейс 1. Сборка и unit-тесты

Проверяет: проект собирается, новые тесты роутера, хендлера и конфигурации
проходят.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON
cmake --build build --parallel
ctest --test-dir build -R 'APIRouter|TaskManagerHandler|ServerConfig' --output-on-failure
```

**Ожидаемый результат:** сборка без ошибок; 16 тестов, `100% tests passed`.

## Кейс 2. Успешный расчёт через сервер

Проверяет: `POST /h1/task` возвращает HTTP 200 и корректный JSON.

```bash
set -a; . ./.env; set +a
./build/bin/h1Server --host 127.0.0.1 --port 8080 --threads 8 &
SERVER_PID=$!
sleep 1
curl -i -X POST http://127.0.0.1:8080/h1/task \
  -H 'Content-Type: application/json' \
  -d '{"left": 2, "right": 8, "operation": "^"}'
```

**Ожидаемый результат:** `HTTP/1.0 200 OK`, `Content-Type: application/json`,
тело содержит `"operation":"^"`, `"result":256.0` (значение `fromCache`
может быть `true`, если операция уже есть в БД).

## Кейс 3. Ошибка вычисления и невалидный JSON

Проверяет: ошибки возвращаются как HTTP 500 с JSON-полем `error`.

```bash
curl -i -X POST http://127.0.0.1:8080/h1/task \
  -H 'Content-Type: application/json' \
  -d '{"left": 10, "right": 0, "operation": "/"}'
curl -i -X POST http://127.0.0.1:8080/h1/task \
  -H 'Content-Type: application/json' \
  -d 'not-json'
```

**Ожидаемый результат:** оба ответа — `HTTP/1.0 500 Internal Server Error`,
`Content-Type: application/json`, тело вида `{"error":"..."}` с непустым
текстом.

## Кейс 4. Cache hit

Проверяет: повторная операция не выполняется заново и помечается
`"fromCache":true`.

```bash
curl -sS -X POST http://127.0.0.1:8080/h1/task \
  -H 'Content-Type: application/json' \
  -d '{"left": 123456, "right": 654321, "operation": "+"}'
curl -sS -X POST http://127.0.0.1:8080/h1/task \
  -H 'Content-Type: application/json' \
  -d '{"left": 123456, "right": 654321, "operation": "+"}'
```

**Ожидаемый результат:** первый ответ содержит `"fromCache":false`, второй —
`"fromCache":true`; `result` одинаковый.

## Кейс 5. Нагрузка: 5000 запросов, 100 одновременно

Проверяет: сервер не блокируется и не падает под конкурентной нагрузкой.

```bash
python3 loadtest.py --requests 5000 --concurrency 100
```

**Ожидаемый результат:** в блоке `RESULT` — `Errors: 0`,
`Status codes: {200: 5000}`; сервер продолжает работать:

```bash
curl -sS -o /dev/null -w '%{http_code}\n' -X POST http://127.0.0.1:8080/h1/task \
  -H 'Content-Type: application/json' \
  -d '{"left": 7, "right": 6, "operation": "*"}'
```

Ожидаемо: `200`.

## Кейс 6. Конкурентные cache miss с записью в БД

Проверяет: параллельные запросы с новыми операциями проходят через
блокировку `PGConn` без взаимоблокировок.

```bash
python3 - <<'PY'
import concurrent.futures, http.client, json, time
def one(i):
    body = json.dumps({"left": 300000 + i, "right": 5, "operation": "-"}).encode()
    c = http.client.HTTPConnection("127.0.0.1", 8080, timeout=20)
    c.request("POST", "/h1/task", body=body, headers={"Content-Type": "application/json"})
    r = c.getresponse(); r.read(); c.close()
    return r.status
N = 1000
t0 = time.perf_counter()
with concurrent.futures.ThreadPoolExecutor(max_workers=100) as ex:
    codes = list(ex.map(one, range(N)))
print("errors:", sum(c != 200 for c in codes), "elapsed:", round(time.perf_counter()-t0, 2))
PY
```

**Ожидаемый результат:** `errors: 0`; процесс сервера жив, контрольный
`curl` отвечает `200`.

## Кейс 7. Конфигурация CLI/env и `--help`

Проверяет: параметры сервера читаются из CLI и окружения; справка работает
без БД.

```bash
./build/bin/h1Server --help
H1_SERVER_PORT=9090 ./build/bin/h1Server --host 127.0.0.1 &
sleep 1
curl -sS -o /dev/null -w '%{http_code}\n' -X POST http://127.0.0.1:9090/h1/task \
  -H 'Content-Type: application/json' \
  -d '{"left": 1, "right": 2, "operation": "+"}'
kill %1
```

**Ожидаемый результат:** `--help` печатает справку и завершается с кодом 0
без ошибок подключения к БД; сервер на порту 9090 отвечает `200`.

## Кейс 8. deb-пакет и systemd-юнит

Проверяет: серверный бинарник, юнит и оба env-файла входят в пакет.

```bash
make package
dpkg -c build/h1-server_1.0-1_amd64.deb
rm -rf /tmp/h1-ctrl && dpkg -e build/h1-server_1.0-1_amd64.deb /tmp/h1-ctrl
cat /tmp/h1-ctrl/conffiles
sudo apt-get install -y ./build/h1-server_1.0-1_amd64.deb
systemctl is-enabled h1-server
sudo systemctl start h1-server
systemctl is-active h1-server
sudo systemctl stop h1-server
sudo apt-get remove -y h1-server
```

**Ожидаемый результат:** в пакете есть только `/usr/bin/h1Server`,
`/usr/lib/systemd/system/h1-server.service`, `/etc/h1/h1-server.env`; нет
`/usr/bin/h1`, `h1.service` и `h1.env`; в `conffiles` — только
`/etc/h1/h1-server.env`; `is-enabled` — `enabled`; после `start` —
`active`; после `stop` — `inactive`; удаление проходит без ошибок.

## Кейс 9. Откат и очистка

Проверяет: окружение возвращается в исходное состояние.

```bash
kill "$SERVER_PID" 2>/dev/null || true
docker compose -f docker/docker-compose.dev.yaml down
git status --short
```

**Ожидаемый результат:** сервер остановлен, контейнер PostgreSQL удалён,
`git status` показывает только ожидаемые файлы change.

---

## Чеклист прогона

- [+] Кейс 1. Сборка и unit-тесты
- [+] Кейс 2. Успешный расчёт через сервер
- [+] Кейс 3. Ошибка вычисления и невалидный JSON
- [+] Кейс 4. Cache hit
- [+] Кейс 5. Нагрузка: 5000 запросов, 100 одновременно
- [+] Кейс 6. Конкурентные cache miss с записью в БД
- [+] Кейс 7. Конфигурация CLI/env и `--help`
- [+] Кейс 8. deb-пакет и systemd-юнит
- [+] Кейс 9. Откат и очистка
