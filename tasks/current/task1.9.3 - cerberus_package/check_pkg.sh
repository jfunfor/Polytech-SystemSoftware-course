#!/bin/bash

fail=0

dpkg -i ./cerberus-telemetry.deb

sleep 2

systemctl is-enabled cerberus.service | grep -q enabled || { echo "Не включён"; fail=1; }
systemctl is-active cerberus.service | grep -q active || { echo "Не запущен"; fail=1; }

journalctl -t cerberus --since "-1 min" | grep -q "Cerberus telemetry" || { echo "Нет логов"; fail=1; }

dpkg -r cerberus-telemetry
sleep 2

systemctl is-active cerberus.service | grep -q inactive || { echo "Не остановлен после удаления"; fail=1; }

if [ "$fail" -eq 0 ]; then
  echo "Всё работает!"
else
  echo "Найдены ошибки"
fi