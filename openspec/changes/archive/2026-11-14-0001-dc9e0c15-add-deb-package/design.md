# Design: add-deb-package

## Context

Проект собирается через CMake 3.22 (clang++, C++20): статическая библиотека
`h1_core` + исполняемый файл `h1` (`cmd/main.cpp`), артефакт `build/bin/h1`.
Внешние зависимости FetchContent (spdlog, nlohmann/json, argparse, libenvpp)
и статическая линковка с ними — текущее поведение по умолчанию; libpq
приходит через `find_package(PostgreSQL REQUIRED)` и линкуется динамически.
В `CMakeLists.txt` уже есть `install(TARGETS ${PROJECT_NAME} ...)` с
`RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}` — устанавливается только
исполняемый файл. Сборочная машина — Ubuntu 22.04 (cpack 3.22.1, dpkg-deb
в наличии). Мотивация и scope — в proposal.md, требования — в
specs/deb-packaging/spec.md.

## Goals / Non-Goals

**Goals:**
- Собрать deb-пакет `h1_<версия>_amd64.deb` средствами CPack из текущего
  CMakeLists.txt, одной командой после обычной сборки.
- Пакет содержит только `/usr/bin/h1`; Depends: `libpq5`.
- Проверяемая установка `apt-get install ./h1_*.deb` на Ubuntu 22.04 и 24.04.
- Автоматизация проверки: контроль состава пакета, метаданных и запуска
  бинарника.

**Non-Goals:**
- Не создается APT-репозиторий (публикация по имени `apt-get install h1`) —
  установка только из локального файла.
- Не создаются пакеты `h1-dev` (заголовки/библиотека), сервисные пакеты
  (.env, docker-compose), скрипты postinst/preinst.
- Не меняется поведение приложения, тестов и текущий способ сборки из
  исходников.
- Не настраивается CI для автоматической публикации пакетов (только локальная
  сборка и, опционально, Makefile target).
- Не решаются вопросы подписи пакета (не требуется для `apt-get install ./файл`).

## Decisions

1. **CPack (generator DEB) внутри существующего CMakeLists.txt** вместо
   `dpkg-buildpackage`/debhelper.
   - Rationale: проект уже целиком на CMake; CPack берет имя/версию из
     `project(h1 VERSION 1.0)`, не требует отдельного каталога `debian/`
     с control/rules/changelog, минимизирует файлы. Debhelper был бы
     «каноничнее» для сообщества, но избыточен для одного бинарника.
   - Реализация: блок `set(CPACK_*)` + `include(CPack)`; зависимость от
     `dpkg-deb`/`dpkg-shlibdeps` (есть в Ubuntu по умолчанию).

2. **Двойная цель сборки пакета**: `cpack` как основная команда + Makefile
   target `package` для удобства (`make package`).
   - Rationale: `cpack` в CMake уже генерирует target `package` при
     `include(CPack)`, но в Makefile проекта приняты краткие алиасы —
     добавляем явный target с документацией через `##`.

3. **Состав пакета — только исполняемый файл**:
   - Использовать существующий `install(TARGETS h1 RUNTIME ...)`;
     из текущего `install()` убрать экспорт-цели, не относящиеся к пакету,
     либо ограничить состав через `CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA`
     / отдельные `install()` только для RUNTIME.
   - Rationale: по ответам пользователя пакет ставит только бинарник;
     include/ и статическая `h1_core` остаются dev-артефактами исходников.

4. **Зависимости FetchContent — статически (как есть)**, объявляем в пакете
   только `Depends: libpq5`.
   - Rationale: сейчас h1_core линкует spdlog/json/argparse/libenvpp
     статически (FetchContent-таргеты), в бинарник они входят автоматически.
     Менять на системные пакеты не требуется и опасно несовместимостью версий.
   - Автоматическое вычисление зависимостей: включить
     `CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON` (dpkg-shlibdeps найдет `libpq5` по
     NEEDED библиотеке `libpq.so.5`), с fallback-объявлением вручную, если
     SHLIBDEPS недоступен. Явно задать `CPACK_DEBIAN_PACKAGE_DEPENDS` как
     минимум, а SHLIBDEPS дополнит.

5. **Метаданные пакета**:
   - `CPACK_PACKAGE_NAME = h1`, `CPACK_PACKAGE_VERSION` — из версии проекта
     (`1.0`); Debian-ревизия `CPACK_DEBIAN_PACKAGE_RELEASE = 1` →
     `h1_1.0-1_amd64.deb` (при пересборках с тем же кодом — increments).
   - `CPACK_DEBIAN_PACKAGE_MAINTAINER` — автор проекта (из заголовков:
     `Mochalov Anton <xoore4ik@gmail.com>`).
   - `CPACK_DEBIAN_PACKAGE_SECTION = utils`,
     `CPACK_DEBIAN_PACKAGE_ARCHITECTURE = amd64`,
     `CPACK_DEBIAN_PACKAGE_DESCRIPTION` — краткое описание из README.
   - `CPACK_PACKAGING_INSTALL_PREFIX = /usr`.

6. **Ubuntu 22.04 и 24.04**:
   - Сборка выполняется на 22.04 (текущая машина) — бинарник собирается
     под libpq этой системы. Проверка на 24.04 выполняется установкой того
     же пакета в контейнер `ubuntu:24.04` (`docker run` + `apt-get install
     ./h1_*.deb`), без отдельной пересборки; отдельные пакеты под релизы
     не делаются, т.к. зависимости разрешаются apt'ом целевой системы.

## Risks / Trade-offs

- [FetchContent требует сети при сборке пакета] → сборка пакета выполняется
  в уже сконфигурированном build-каталоге (зависимости скачаны на этапе
  cmake); это поведение идентично обычной сборке, ничего нового не добавляет.
- [SHLIBDEPS может не вычислить зависимости на минимальных системах] →
  явный минимум `CPACK_DEBIAN_PACKAGE_DEPENDS "libpq5"`; SHLIBDEPS только
  дополняет список.
- [Бинарник, собранный на 22.04, может тянуть свежие версии libc/libstdc++] →
  проверка через `dpkg -I` и тестовый запуск в контейнере 24.04 входит в
  задачи; при несовместимости — пересборка на целевом релизе в контейнере
  (шаг описан в tasks как опциональный).
- [Conflicts при повторной сборке с той же версией] → `apt-get install
  ./файл.deb` с той же версией требует `--reinstall` или bump
  `CPACK_DEBIAN_PACKAGE_RELEASE`; зафиксировать это в README.
- [Существующий `install(TARGETS ... EXPORT ...)` может вытянуть в пакет
  лишнее] → перед сборкой проверить состав `dpkg -c` и ограничить
  устанавливаемые артефакты только RUNTIME.

## Migration Plan

1. Добавить блок CPack в `CMakeLists.txt` (после существующих `install()`).
2. Пересобрать и запустить `cpack`/`make package` → получить deb.
3. Проверить состав и метаданные (`dpkg -c`, `dpkg -I`), установить локально
   через `apt-get install ./h1_*.deb`, проверить `h1 --help`.
4. Откат — обычный `git revert` изменений CMakeLists.txt/Makefile;
   удаление пакета — `apt-get remove h1`.

## Open Questions

- (нет — ключевые вопросы согласованы с пользователем на этапе proposal:
  состав пакета, способ установки, CPack, Depends libpq5, платформы,
  статические FetchContent-зависимости)
