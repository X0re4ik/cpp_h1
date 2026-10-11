#!/usr/bin/env bash
mkdir -p ./logs


JSON_DATA=$(cat ./scripts/data/big.json)
BIN_PATH="./build/bin/h1"

echo "Проверка нагрузки CPU"
sudo -E perf stat -- \
    $BIN_PATH \
    --verbose \
    --config "$JSON_DATA"

echo "Отчёт по нагрузке на CPU блоков программы"
sudo -E perf record \
    --output ./logs/perf.data \
    -- \
    $BIN_PATH \
    --verbose \
    --config "$JSON_DATA"

echo "Открываю отчёт"
sudo -E perf report --input ./logs/perf.data