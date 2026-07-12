# Форматирование кода


1. Конфигурация
Выполнить сборку проекта с флагом `USE_CLANG_FORMAT_FIX` 
```bash
cmake -S . -B build -DUSE_CLANG_FORMAT_FIX=ON
```

2. Сборка
Запустить форматирование кода проекта локально
```bash
cmake --build build --target clang-format-fix
```
