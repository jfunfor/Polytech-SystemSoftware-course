#!/bin/bash

fail=0

# Проверка наличия юнит-файла
test -f /etc/systemd/system/plasma-coffee.service || { echo "Юнит-файл отсутствует"; fail=1; }

# Проверка, что включен
systemctl is-enabled plasma-coffee.service | grep -q enabled || { echo "Сервис не включен"; fail=1; }

# Проверка, что активен
systemctl is-active plasma-coffee.service | grep -q active || { echo "Сервис не активен"; fail=1; }

# Проверка логов
grep -q "Плазменный кофе готов" /var/log/plasma-coffee.log || { echo "Лог не найден"; fail=1; }

# Проверка имени пользователя в логе
grep -q "bitreaper" /var/log/plasma-coffee.log || { echo "Неверный пользователь в логе"; fail=1; }

# Результат
if [ "$fail" -eq 0 ]; then
  echo "Все проверки пройдены"
  exit 0
else
  echo "Ошибки найдены"
  exit 1
fi