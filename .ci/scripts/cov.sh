#!/bin/bash

BUILD_TEST_PATH="./build/test"
TEST_BIN_PATH="./build/bin/h1_test"

mkdir -p ${BUILD_TEST_PATH}

# Запуск тестов с фиксацийе данных в .profraw
LLVM_PROFILE_FILE="${BUILD_TEST_PATH}/h1_test.profraw" ${TEST_BIN_PATH}

# Конвертация результатов тестирования в .profdata
echo "Конвертация результатов тестирования в .profdata"
llvm-profdata merge \
	-sparse \
	${BUILD_TEST_PATH}/h1_test.profraw \
	-o ${BUILD_TEST_PATH}/h1_test.profdata

# Формирование html документа
echo "Формирование html документа"
llvm-cov show \
	${TEST_BIN_PATH} \
	-instr-profile=${BUILD_TEST_PATH}/h1_test.profdata \
	-format=html \
	-output-dir=${BUILD_TEST_PATH}/coverage_html \
	-show-line-counts-or-regions \
	-show-branches=percent \
	src/

# # Конвертация в coverage.info для отправки данных в SonarQube
# llvm-cov export ./<path_to_test> \
#     -format=lcov \
#     -instr-profile test.profdata > coverage.info
