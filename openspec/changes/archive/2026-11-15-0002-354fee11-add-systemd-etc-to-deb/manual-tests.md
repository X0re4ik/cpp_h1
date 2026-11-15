# Ручные проверочные кейсы: 0002-354fee11-add-systemd-etc-to-deb

Самодостаточная инструкция для человека. Прогон кейсов даёт уверенность,
что пакетная механика (systemd-юниты + конфиг `/etc/h1/h1.env`) работает
на реальной системе.

## Пререквизиты

- Собранный пакет: `build/h1_1.0-1_amd64.deb` (пересобрать: `make package`).
- Права `sudo` (установка/удаление пакета, systemctl).
- Работающий systemd (обычный десктоп/сервер Ubuntu; НЕ WSL1, НЕ контейнер
  без systemd).
- Доступный PostgreSQL с параметрами, которые впишете в `/etc/h1/h1.env`
  (например: `docker compose -f docker/docker-compose.dev.yaml up -d`).
- Кейс 8 требует двух версий пакета: перед его выполнением не удаляйте
  старый пакет, если он установлен; либо сначала поставьте пакет из
  предыдущей ревизии (`CPACK_DEBIAN_PACKAGE_RELEASE`), потом пересоберите
  новую.

---

## Кейс 1. Состав пакета

Проверяет: требование «Состав пакета» — бинарник, юнит, conffile; ничего
лишнего.

```bash
dpkg -c build/h1_1.0-1_amd64.deb
dpkg -e build/h1_1.0-1_amd64.deb /tmp/h1-ctrl && cat /tmp/h1-ctrl/conffiles
```

**Ожидаемый результат:** в пакете есть `./usr/bin/h1`,
`./usr/lib/systemd/system/h1.service`, `./etc/h1/h1.env`; ничего другого
(кроме служебных каталогов). В `conffiles` — строка `/etc/h1/h1.env`.
В `/tmp/h1-ctrl` присутствуют `postinst`, `prerm`, `postrm`, `conffiles`.

## Кейс 2. Установка: юнит появляется и включается в автозагрузку

Проверяет: сценарии «Юнит виден сразу после установки» и «Юнит в
автозагрузке после установки» (без ручного `daemon-reload`).

```bash
sudo apt-get install ./build/h1_1.0-1_amd64.deb
systemctl cat h1
systemctl is-enabled h1
```

**Ожидаемый результат:** установка без ошибок; `systemctl cat h1` показывает
юнит (файл `/lib/systemd/system/h1.service` с `EnvironmentFile=
-/etc/h1/h1.env`); `systemctl is-enabled h1` отвечает `enabled`.

## Кейс 3. Разовый расчёт через systemctl

Проверяет: требование «Разовый расчёт через systemd-юнит». Перед запуском
впишите в `/etc/h1/h1.env` реальные параметры БД:

```bash
sudoedit /etc/h1/h1.env
sudo systemctl start h1
systemctl status h1
journalctl -u h1 --since "5 minutes ago"
```

**Ожидаемый результат:** юнит отрабатывает и переходит в
`inactive (dead)` / «Deactivated successfully» без ошибок; в журнале виден
результат расчёта (например `2+2=4.00`) и NOTICE о создании схемы `h1`.
Повторный `systemctl start h1` снова выполняет расчёт.

## Кейс 4. Удаление: юнит исчезает, автозагрузка снята

Проверяет: сценарии «Юнит исчезает сразу после удаления» и «Автозагрузка
снята при удалении». (После кейса 7 верните пакет установкой обратно.)

```bash
sudo apt-get remove h1
ls /usr/lib/systemd/system/h1.service 2>&1
systemctl is-enabled h1 2>&1
systemctl status h1 2>&1 | head -3
```

**Ожидаемый результат:** файл юнита отсутствует (`No such file or
directory`); `is-enabled` НЕ отвечает `enabled` (disabled/not-found);
`systemctl status` не показывает активный юнит и не требует ручного
`daemon-reload`.

## Кейс 5. Отсутствие файла окружения

Проверяет: сценарий «Юнит работает без файла окружения» — старт юнита не
блокируется, ошибка подключения видна в журнале, coredump допустим
(исправление — отдельный change). Выполнять при установленном пакете.

```bash
sudo mv /etc/h1/h1.env /etc/h1/h1.env.bak
sudo systemctl start h1
systemctl status h1 | head -6
journalctl -u h1 --since "2 minutes ago" | tail -5
sudo mv /etc/h1/h1.env.bak /etc/h1/h1.env
```

**Ожидаемый результат:** юнит стартует (start не отклонён); приложение
падает из-за отсутствующих `DB_*`-переменных — в журнале видна ошибка вида
`DB_PGHOST: Environment variable 'DB_PGHOST' not set` (возможен
`code=dumped, status=6/ABRT`); файл возвращён на место.

## Кейс 6. Отключение юнита из автозагрузки вручную

Проверяет: коннотацию требования «Автовключение» — пользователь может
отключить автозапуск.

```bash
sudo systemctl disable h1
systemctl is-enabled h1
sudo systemctl enable h1
systemctl is-enabled h1
```

**Ожидаемый результат:** после disable — `disabled`; после enable —
`enabled`. Обе операции без ошибок.

## Кейс 7. Новый юнит попадает в пакет без правок CMake

Проверяет: требование «Автоматическое включение systemd-юнитов в пакет».

```bash
cat > packaging/systemd/h1-test.timer <<'EOF'
[Unit]
Description=h1 test timer (package check)

[Timer]
OnCalendar=daily

[Install]
WantedBy=timers.target
EOF
make package
dpkg -c build/h1_1.0-1_amd64.deb | grep h1-test
```

**Ожидаемый результат:** в пакете появился `./usr/lib/systemd/system/
h1-test.timer` без каких-либо правок CMakeLists.txt. После проверки
удалить файл и пересобрать пакет:

```bash
rm packaging/systemd/h1-test.timer && make package
dpkg -c build/h1_1.0-1_amd64.deb | grep h1-test
```

**Ожидаемый результат:** таймер снова отсутствует в пакете.

## Кейс 8. Conffile: правки сохраняются при обновлении

Проверяет: требование «Конфигурация окружения как conffile». Требует
установленной старой версии пакета (без правок в `/etc/h1/h1.env` —
см. пререквизиты).

```bash
sudoedit /etc/h1/h1.env          # изменить, например, DB_PGHOST=localhost -> db.local
sudo apt-get install ./build/h1_1.0-1_amd64.deb   # установка "новой" ревизии
grep DB_PGHOST /etc/h1/h1.env
```

**Ожидаемый результат:** dpkg сообщает о конфликте conffile
(`changed config file ...`) и спрашивает решение (N — оставить вашу
версию, Y — пакетную). При выборе «оставить локальную» правка
`DB_PGHOST=db.local` сохраняется.

## Кейс 9. Дублирование уже пройденных автоматических проверок

Проверка составов уже выполнялась агентом (задачи 3.1/3.2: состав пакета
и поведение опции `-DH1_INSTALL_SYSTEMD_UNITS=OFF`), дублировать вручную
не обязательно — кейс оставлен для полноты.

```bash
sudo apt-get remove h1 && sudo apt-get install ./build/h1_1.0-1_amd64.deb && systemctl is-enabled h1
```

**Ожидаемый результат:** после переустановки юнит снова `enabled`.

---

## Чеклист прогона

- [+] Кейс 1. Состав пакета
- [+] Кейс 2. Установка: юнит появляется и включается в автозагрузку
- [+] Кейс 3. Разовый расчёт через systemctl
- [+] Кейс 4. Удаление: юнит исчезает, автозагрузка снята
- [+] Кейс 5. Отсутствие файла окружения
- [+] Кейс 6. Отключение/включение юнита вручную
- [+] Кейс 7. Новый юнит попадает в пакет без правок CMake
- [+] Кейс 8. Conffile: правки сохраняются при обновлении
- [+] Кейс 9. Переустановка: юнит снова enabled
