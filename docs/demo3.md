1) Демострация работы КЭШ:

(miss)
```bash
./build/bin/h1 --config '{"left": 2, "right": 8, "operation": "^"}' --verbose
```
(hit)
```bash
./build/bin/h1 --config '{"left": 2, "right": 8, "operation": "^"}' --verbose
```
(miss)
```bash
./build/bin/h1 --config '{"left": 2, "right": 9, "operation": "^"}' --verbose
```
(miss)
```bash
./build/bin/h1 --config '{"left": 2, "right": 0, "operation": "/"}' --verbose
```
(hit)
```bash
./build/bin/h1 --config '{"left": 2, "right": 0, "operation": "/"}' --verbose
```

2) Запуск Valgrind:
```bash
./scripts/valgrid-smock-tests.sh
```

3) Запуск tests с valgrind через cmake:
```bash
mamke memcheck
```

4) Запуск perf для анализа производительности:
```bash
./scripts/perf-test.sh
```