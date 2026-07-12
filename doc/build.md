# Clang tools

Установка clang-format:
```bash
wget https://apt.llvm.org/llvm.sh
chmod +x llvm.sh
sudo ./llvm.sh 21
sudo apt install -y clang-format-21
```

Установка run-clang-format.py:
```bash
wget https://raw.githubusercontent.com/Sarcasm/run-clang-format/master/run-clang-format.py
chmod +x run-clang-format.py
sudo mv run-clang-format.py /usr/local/bin/
```

# Сборка проекта
Конфигурируем проект:
```bash
cmake -B build -G "Ninja" -DUSE_CLANG_FORMAT=ON
```

Описание параметров конфигурации:
* `-DUSE_CLANG_FORMAT`: `ON` - выполняется автоматическое форматирование кода, `OFF` - не выполняется

Собираем проект:
```bash
cmake --build build
```

Локальный запуск clang-format
```bash
cmake --build build --target clang-format-fix
```
