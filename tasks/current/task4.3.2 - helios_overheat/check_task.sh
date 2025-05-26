#!/bin/bash

base_dir="/home/student/helios_stack"
score=0

echo "== Проверка задания: Перегрев в дата-кубе Гелиос =="

# 1. Проверка metrics_types_report.txt
if grep -iq "counter" "$base_dir/metrics_types_report.txt" &&
   grep -iq "gauge" "$base_dir/metrics_types_report.txt" &&
   grep -iq "histogram" "$base_dir/metrics_types_report.txt" &&
   grep -iq "summary" "$base_dir/metrics_types_report.txt"; then
  echo "Метрики перечислены"
  ((score++))
else
  echo "Не все типы метрик найдены"
fi

# 2. Проверка изменения scrape_interval < 5s → 15s
if grep -q 'scrape_interval: 15s' "$base_dir/prometheus.yml" &&
   ! grep -q 'scrape_interval: 2s' "$base_dir/prometheus.yml"; then
  echo "Интервалы scrape оптимизированы"
  ((score++))
else
  echo "Ошибки в интервалах scrape"
fi

# 3. Проверка Federation комментария
if grep -q '# Federation enabled' "$base_dir/prometheus.yml"; then
  echo "Federation комментарий найден"
  ((score++))
else
  echo "Комментарий о federation отсутствует"
fi

# 4. Проверка heavy_panels.txt
if grep -q '^[0-9]\+$' "$base_dir/heavy_panels.txt"; then
  echo "Найдены тяжёлые панели"
  ((score++))
else
  echo "Панели не найдены или файл пуст"
fi

# 5. Проверка полноты алертов
alerts_pass=1
for alert_file in "$base_dir/alerts/"*.yml; do
  for field in expr for labels annotations; do
    if ! grep -q "$field:" "$alert_file"; then
      alerts_pass=0
      echo "Не хватает поля '$field' в $alert_file"
    fi
  done
done

if [ "$alerts_pass" -eq 1 ]; then
  echo "Все алерты корректны"
  ((score++))
fi

echo "== Результат: $score из 5 баллов =="

if [ $score -eq 5 ]; then
  echo "Задание выполнено полностью и правильно!"
else
  echo "Проверь ошибки и повтори попытку."
fi