


SHELL := /usr/bin/env bash

PROJECT_NAME := h1
BUILD_DIR := build

.DEFAULT_GOAL := help
.PHONY: help configure build rebuild run test memcheck format tidy perf valgrind clean

help: ## Показать список доступных команд
	@printf '\nИспользование: make <команда>\n\n'
	@awk 'BEGIN {FS = ":.*## "} /^[a-zA-Z0-9_-]+:.*## / {printf "  %-12s %s\n", $$1, $$2}' $(MAKEFILE_LIST)
	@printf '\n'

configure: ## Сконфигурировать Debug-сборку с тестами
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON

build: configure ## Собрать приложение и тесты
	cmake --build $(BUILD_DIR) --parallel

rebuild: ## Полностью пересобрать проект
	cmake --build $(BUILD_DIR) --clean-first --parallel

run: build ## Запустить приложение (передайте ARGS='...')
	./$(BUILD_DIR)/bin/$(PROJECT_NAME) $(ARGS)

test: build ## Запустить тесты через CTest
	ctest --test-dir $(BUILD_DIR) --output-on-failure

memcheck: build ## Запустить CTest Memcheck
	ctest --test-dir ./build -T memcheck

format: ## Отформатировать исходный код
	cmake -S . -B $(BUILD_DIR) -DUSE_CLANG_FORMAT_FIX=ON
	cmake --build $(BUILD_DIR) --target clang-format-fix

tidy: ## Запустить Clang-Tidy
	cmake -S . -B $(BUILD_DIR) -DBUILD_TESTING=ON -DUSE_CLANG_TIDY=ON
	cmake --build $(BUILD_DIR) --target clang-tidy-check

perf: ## Запустить Perf-профилирование
	./scripts/perf-test.sh

valgrind: ## Запустить Valgrind-сценарий
	./scripts/valgrid-smock-tests.sh

clean: ## Удалить артефакты сборки
	cmake --build $(BUILD_DIR) --target clean 2>/dev/null || true
