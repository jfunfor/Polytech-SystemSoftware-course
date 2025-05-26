#!/bin/bash

base_dir="/home/neuro_logs"
score=0

echo "== Проверка задания: Потерянный узел в Синтетик-Сити =="

# 1. Scrape в логах
if grep -q 'Scrape succeeded.*node-α45.cortex.net' "$base_dir/prometheus_logs/prometheus.log"; then
  echo "Scrape найден"
  ((score++))
else
  echo "Scrape не найден"
fi

# 2. Prometheus конфиг содержит нужный target
if grep -q 'node-α45.cortex.net' "$base_dir/prometheus.yml"; then
  echo "Узел добавлен в Prometheus"
  ((score++))
else
  echo "Узел не найден в Prometheus конфиге"
fi

# 3. Alert в alerts.yml
if grep -q 'alert: NodeDown_Alpha45' "$base_dir/alerts.yml" && grep -q 'up{instance="node-α45.cortex.net"} == 0' "$base_dir/alerts.yml"; then
  echo "Алерт NodeDown_Alpha45 настроен"
  ((score++))
else
  echo "Алерт не найден или неправильно настроен"
fi

# 4. Alertmanager route и email
if grep -q 'receiver: sys_admins' "$base_dir/alertmanager.yml" && grep -q 'to: .*admins@neuropulse.cy' "$base_dir/alertmanager.yml"; then
  echo "Роутинг и email настроены"
  ((score++))
else
  echo "Ошибка в настройке alertmanager.yml"
fi

echo "== Результат: $score из 4 баллов =="

if [ $score -eq 4 ]; then
  echo "Задание выполнено успешно!"
else
  echo "Исправь ошибки и попробуй снова."
fi