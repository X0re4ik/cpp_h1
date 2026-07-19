#!/usr/bin/env bash
#
# Набор smoke/regression тестов для приложения h1 (CLI-калькулятор).
#
# Зелёным помечаются успешно пройденные проверки, красным — проваленные.
# Тесты фиксируют ОЖИДАЕМОЕ (корректное) поведение программы, поэтому пока
# известные баги не исправлены, часть тестов будет падать — это нормально
# и как раз показывает, что именно сломано.
#
# Контракт приложения: exit code ВСЕГДА 0 (в том числе при ошибках), но
# при ошибке (деление на 0, некорректные/пустые значения и т.п.) программа
# обязана вывести сообщение об ошибке, а не падать и не молчать.
#
# Использование:
#   ./tests/run_tests.sh              # пересобрать проект и прогнать тесты
#   ./tests/run_tests.sh --no-build   # прогнать тесты на уже собранном бинарнике
#

set -u

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_ROOT/build"
BINARY="$BUILD_DIR/bin/h1"

GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[0;33m'
BOLD='\033[1m'
NC='\033[0m'

PASS_COUNT=0
FAIL_COUNT=0

SKIP_BUILD=0
if [[ "${1:-}" == "--no-build" ]]; then
    SKIP_BUILD=1
fi

if [[ "$SKIP_BUILD" -eq 0 ]]; then
    echo "== Сборка проекта =="
    if ! cmake -S "$PROJECT_ROOT" -B "$BUILD_DIR" >/tmp/opencode_cmake_configure.log 2>&1; then
        echo -e "${RED}Ошибка конфигурации CMake. Смотри /tmp/opencode_cmake_configure.log${NC}"
        cat /tmp/opencode_cmake_configure.log
        exit 1
    fi
    if ! cmake --build "$BUILD_DIR" >/tmp/opencode_cmake_build.log 2>&1; then
        echo -e "${RED}Сборка не удалась. Смотри /tmp/opencode_cmake_build.log${NC}"
        cat /tmp/opencode_cmake_build.log
        exit 1
    fi
    echo
fi

if [[ ! -x "$BINARY" ]]; then
    echo -e "${RED}Бинарник не найден: $BINARY${NC}"
    exit 1
fi

# run_test <название> <ожидаемый_exit: zero|nonzero|any> <ожидаемое_в_выводе: "a;b;c"|NONEMPTY|""> -- <аргументы...>
run_test() {
    local name="$1"
    local expect_exit="$2"
    local expect_contains="$3"
    shift 3
    if [[ "${1:-}" == "--" ]]; then
        shift
    fi
    local args=("$@")

    local output
    local actual_exit
    output="$("$BINARY" "${args[@]}" 2>&1)"
    actual_exit=$?

    local ok=1
    local reasons=()

    if [[ "$actual_exit" -ge 128 ]]; then
        ok=0
        reasons+=("ПРОГРАММА УПАЛА по сигналу (exit=$actual_exit, похоже на segfault/abort)")
    fi

    case "$expect_exit" in
        zero)
            if [[ "$actual_exit" -ne 0 ]]; then
                ok=0
                reasons+=("ожидался exit code 0, получено $actual_exit")
            fi
            ;;
        nonzero)
            if [[ "$actual_exit" -eq 0 ]]; then
                ok=0
                reasons+=("ожидался ненулевой exit code, получено 0")
            fi
            ;;
        any) ;;
    esac

    if [[ "$expect_contains" == "NONEMPTY" ]]; then
        if [[ -z "$(tr -d '[:space:]' <<<"$output")" ]]; then
            ok=0
            reasons+=("ожидался непустой вывод, но вывод пуст")
        fi
    elif [[ -n "$expect_contains" ]]; then
        local IFS=';'
        local -a needles=($expect_contains)
        unset IFS
        local needle
        for needle in "${needles[@]}"; do
            if ! grep -qF -- "$needle" <<<"$output"; then
                ok=0
                reasons+=("в выводе не найдено: '$needle'")
            fi
        done
    fi

    if [[ "$ok" -eq 1 ]]; then
        printf "${GREEN}[ OK ]${NC} %s\n" "$name"
        PASS_COUNT=$((PASS_COUNT + 1))
    else
        printf "${RED}[FAIL]${NC} %s\n" "$name"
        local r
        for r in "${reasons[@]}"; do
            printf "       ${YELLOW}- %s${NC}\n" "$r"
        done
        printf "       вывод программы: %s\n" "${output:-<пусто>}"
        FAIL_COUNT=$((FAIL_COUNT + 1))
    fi
}

echo "== Запуск тестов =="
echo

run_test "Сложение: 2 + 3 = 5"                        zero "Result: 5"                                   -- -a 2 3
run_test "Вычитание: 10 - 4 = 6"                      zero "Result: 6"                                   -- -s 10 4
run_test "Умножение: 3 * 4 = 12"                      zero "Result: 12"                                  -- -m 3 4
run_test "Деление: 10 / 2 = 5"                        zero "Result: 5"                                   -- -d 10 2
run_test "Деление на 0 -> exit 0 и сообщение об ошибке" zero "ERROR"                                      -- -d 5 0
run_test "Факториал 5! = 120"                         zero "Result: 120"                                 -- -f 5
run_test "Факториал 0! = 1"                           zero "Result: 1"                                   -- -f 0

run_test "Возведение в степень 0^2 = 0"               zero "Result: 0"                                   -- -p 0 2
run_test "Возведение в степень 1^0 = 1"               zero "Result: 1"                                   -- -p 1 0
run_test "Возведение в степень 2^5 = 32"              zero "Result: 32"                                  -- -p 2 5

run_test "Факториал отрицательного -> сообщение"      zero "ERROR"                                       -- -f -5
run_test "Деление без второго аргумента -> сообщение" zero "ERROR"                                       -- -d 5
run_test "Неизвестный флаг -> сообщение, не тихий счёт" zero "ERROR"                                      -- -x 2 3

run_test "Без аргументов -> есть сообщение"           zero "NONEMPTY"                                    --
run_test "--help содержит все опции"                  zero "--add;--sub;--mul;--div;--factorial;--help" -- -h

echo
echo "== Итог =="
printf "${GREEN}Успешно: %d${NC}\n" "$PASS_COUNT"
printf "${RED}Провалено: %d${NC}\n" "$FAIL_COUNT"
echo

if [[ "$FAIL_COUNT" -gt 0 ]]; then
    printf "${RED}${BOLD}ЕСТЬ ПРОВАЛЕННЫЕ ТЕСТЫ${NC}\n"
    exit 1
fi

printf "${GREEN}${BOLD}ВСЕ ТЕСТЫ ПРОЙДЕНЫ${NC}\n"
exit 0
