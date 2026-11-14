# Контекст проекта h1 — карта артефактов

Проект: консольный калькулятор C++20 (учебный, ДЗ-2/ДЗ-3). Принимает операцию
в виде JSON-строки через CLI (`--config '{"left": 2, "right": 3, "operation": "+"}'`),
считает, печатает результат, сохраняет историю в PostgreSQL и кеш.

Эта папка — системные артефакты проекта. Перед разработкой спеки прочитай
нужные файлы (все пути от корня репозитория):

| Файл | Когда читать |
|---|---|
| `openspec/context/architecture.md` | всегда: модули, namespace, поток выполнения, зависимости |
| `openspec/context/coding-standards.md` | всегда: правила формирования .hpp/.cpp, стиль, именование |
| `openspec/context/build-and-verification.md` | перед планированием задач проверки: сборка, тесты, форматирование, tidy, valgrind |
| `openspec/context/constraints.md` | всегда: обязательные ограничения заданий (запреты) |
| `openspec/context/change-workflow.md` | перед созданием нового change: имена-миграции `NNNN-<hash>-<name>`, нумерация строго по возрастанию, уникальный хеш |

## Основные источники в репозитории

- `README.md` — описание, примеры запуска, архитектура, команды.
- `docs/task2.md` — задание ДЗ-2 (ООП, nlohmann/json, GTest, spdlog через обертку).
- `docs/task3.md` — задание ДЗ-3 (PostgreSQL + собственная обертка над libpq,
  кеш, RAII, запреты). Концепт-код приложения из задания — в этом файле.
- `docs/task3_check.md` — итоги проверки ДЗ-3 и известные недоработки
  (тесты БД/кеша, коммутативность "1+2"=="2+1", лимит прогрева кеша и т.п.).
  Хорошая отправная точка для новых изменений.
- `CMakeLists.txt` — список исходников `H1_CORE_SOURCES`, зависимости, тесты,
  custom targets (clang-format-fix, clang-tidy-check, valgrind-tests).
- `Makefile` — краткие команды (`make help`).
- `.clang-format`, `.clang-tidy` — стиль и статический анализ.
- `.env` / `.env.example` — переменные окружения PostgreSQL (`DB_PG*`).
- `docker/docker-compose.dev.yaml` — PostgreSQL 16 для разработки.
- `scripts/perf-test.sh`, `scripts/valgrid-smock-tests.sh` — Perf и Valgrind.
- Тесты: `src/test_*.cpp` (GoogleTest).
