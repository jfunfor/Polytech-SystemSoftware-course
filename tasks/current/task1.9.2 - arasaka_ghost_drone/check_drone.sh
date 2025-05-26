#!/bin/bash

fail=0

# 1. Проверка списка
test -f /tmp/running_services.log || { echo "Нет лога активных сервисов"; fail=1; }

# 2. Проверка остановки дрона
systemctl is-active arasaka-drone.service | grep -q inactive || { echo "Сервис дрона активен"; fail=1; }

# 3. Проверка отключения автозапуска
systemctl is-enabled arasaka-drone.service | grep -q disabled || { echo "Сервис дрона включен"; fail=1; }

# 4. Проверка остановки зависимого сервиса
systemctl is-active drone-sensor.service | grep -q inactive || { echo "Зависимый сервис активен"; fail=1; }

# 5. Проверка логов на отсутствие новых сообщений
recent_logs=$(journalctl -t arasaka-drone --since "5 minutes ago" | grep "Telemetry uplink OK")
if [ -n "$recent_logs" ]; then
  echo "Обнаружены свежие логи от дрона"
  fail=1
fi

# Финал
if [ "$fail" -eq 0 ]; then
  echo "Все проверки пройдены"
  exit 0
else
  echo "Задание не выполнено полностью"
  exit 1
fi