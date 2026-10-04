#!/usr/bin/env bash

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ENV_FILE="${PROJECT_ROOT}/.env"

if [[ ! -f "${ENV_FILE}" ]]; then
    echo "Не найден файл: ${ENV_FILE}" >&2
    return 1 2>/dev/null || exit 1
fi

set -a
source "${ENV_FILE}"
set +a

echo "Переменные из .env загружены."