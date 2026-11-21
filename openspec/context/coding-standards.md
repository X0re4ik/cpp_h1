# Правила формирования кода (.hpp и .cpp)

## Заголовочные файлы (.hpp)

1. Публичные заголовки живут ТОЛЬКО в `include/h1/<module>/`. Include-путь
   начинается с `h1/...` (каталог `include/` подключен как PUBLIC include dir
   цели `h1_core`).
2. Include guard: `#ifndef H_H1_<MODULE>_<HASH>` / `#define ...` / `#endif`
   (hash — md5-подобный суффикс; смотри существующие файлы). Ряд файлов
   дополнительно содержит `#pragma once` первой строкой; для нового кода
   основной стиль — guard.
3. Порядок include (clang-format регруппирует автоматически, IncludeBlocks:
   Regroup):
   - соседние заголовки модуля: `"./xxx.hpp"`;
   - проектные: `"h1/..."`;
   - системные C-API: `<libpq-fe.h>`;
   - стандартные: `<memory>`, `<string>`, `<optional>` и т.д.
4. Общие алиасы и исключения — из `h1/common.hpp`: `h1::String`,
   `h1::H1BaseException`, макросы `H1_NODISCARD` (`[[nodiscard]]`),
   `H1_NOEXCEPT`, `H1_EXCEPT`, `H1_DISCARD`, `H1_MAYBE_UNUSED`,
   `H1_DEFAULT_WHAT_METHOD`. Использовать их вместо прямых атрибутов.
5. Namespace: `h1::<module>` (`h1::calc`, `h1::db::pg`, `h1::lcache`, ...),
   закрывающий комментарий `} // namespace h1::<module>`. Контент внутри
   namespace не индентируется (NamespaceIndentation: None).
6. Классы:
   - интерфейсы `I<Name>`: виртуальный деструктор `= default`, явно указанные
     special member functions (copy/move default или delete);
   - приватные члены данных с суффиксом `_`;
   - вложенные типы допускаются (например `BaseSimpleCalculator::Task`).
7. Фабрики: свободные функции `make*` / `init*`, возвращающие
   `using XxxPtr = std::unique_ptr<IXxx>`. Сырые указатели наружу не отдавать.
8. Именование (проверяется clang-tidy readability-identifier-naming):
   классы/структуры — CamelCase, методы/переменные/параметры — camelBack,
   namespace — lower_case, статические константы — camelBack с суффиксом `_`.
9. Кастомные исключения — наследники `h1::H1BaseException`, отдельный файл
   `<module>/exceptions.hpp` на модуль.
10. Комментарии к методам интерфейсов — по стилю существующего кода
    (`// NOTE: ...`, Doxygen `@brief` у файловых шапок допустим).

## Файлы реализации (.cpp)

1. Реализация зеркалит структуру include: `include/h1/<module>/x.hpp` →
   `src/<module>/x.cpp`. Первый include — собственный заголовок:
   `#include "h1/<module>/x.hpp"`, затем проектные, затем системные/std.
2. Файловые локали (детали реализации, deleter'ы, хелперы) — в анонимном
   namespace вверху файла.
3. Определения методов — полное квалифицированное имя
   (`h1::calc::AdditionCalculator::calculate()`), переносы сигнатуры по
   clang-format.
4. RAII обязателен: внешние C-ресурсы (`PGconn`, `PGresult`, `Task` из
   cpp_h1_lib) только через `std::unique_ptr` с кастомным deleter'ом;
   владение между модулями — умными указателями. Сырые указатели допустимы
   только на границе C-API (libpq, `main(int argc, char* argv[])`).
5. Ошибки — через исключения (`calc::CalculatorException`,
   `H1BaseException`-наследники). Логирование — только через макросы
   `H1_LOG_INFO/H1_LOG_DEBUG/H1_LOG_WARN/H1_LOG_ERROR` (не использовать
   spdlog напрямую).
6. НОЛЬ многопоточности (запрещено заданием).
7. Тесты — отдельные файлы `src/test_<module>.cpp` (GoogleTest, gtest/gmock).
   Каждый новый тестовый файл добавляется в `add_executable(h1_test ...)`
   в `CMakeLists.txt`.
8. После правок кода запускать форматирование и clang-tidy (см.
   `build-and-verification.md`). clang-tidy: все warnings = errors
   (WarningsAsErrors: '*'), смещения отключенных проверок — в `.clang-tidy`.

## Формат (`.clang-format`)

ColumnLimit 80, IndentWidth 4, TabWidth 4 (UseTab: Never), скобки Allman
(`{` с новой строки, AfterClass/AfterFunction/AfterNamespace), доступ к
членам класса с отступом +2 (`public:`/`private:` с отступом 2),
PointerAlignment: Left (`int* p`), SortIncludes: true,
FixNamespaceComments: true.
