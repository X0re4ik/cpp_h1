#!/usr/bin/env bash


TEST_CASE_NUMBER=1

valgrind_h1_run() {
    echo "Test #${TEST_CASE_NUMBER}"
    local config="${1:?укажите config как аргумент}"

    valgrind \
        --tool=memcheck \
        --leak-check=full \
        --show-leak-kinds=all \
        --track-origins=yes \
        --log-file=./logs/valgrind.%p.log \
        --gen-suppressions=all \
        --error-exitcode=42 \
        ./build/bin/h1 \
        --verbose \
        --config "$config"

    status=$?

    echo "Test #${TEST_CASE_NUMBER} OK. Status Code: ${status}"
    TEST_CASE_NUMBER=$((TEST_CASE_NUMBER + 1))
}


valgrind_h1_run '{"left": 1, "right": 1, "operation": "/"}'
valgrind_h1_run '{"left": 1, "right": 0, "operation": "/"}'
valgrind_h1_run '{"left": 1, "right": 0, "operation": "+"}'
valgrind_h1_run '{"left": 1, "right": 0, "operation": "-"}'
valgrind_h1_run '{"left": 1, "right": 0, "operation": "*"}'
valgrind_h1_run '{"left": 1, "right": 6, "operation": "^"}'