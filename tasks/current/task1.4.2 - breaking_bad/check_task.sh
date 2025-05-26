#!/bin/bash

# 1. Выполним скрипт
chmod +x node_report.sh
./node_report.sh --input manifest.json > result.txt

# 2. Проверка количества строк
expected_lines=2
actual_lines=$(wc -l < result.txt)
if [ "$actual_lines" -ne "$expected_lines" ]; then
  echo "Ожидалось $expected_lines строки(ок), найдено $actual_lines"
  exit 1
fi

# 3. Проверка наличия конкретных ID
if ! grep -q "core.1" result.txt || ! grep -q "auth.7" result.txt; then
  echo "Не найдены активные узлы в выводе"
  exit 1
fi

# 4. Проверка сортировки по убыванию
priority_first=$(awk -F'|' '{print $2}' result.txt | head -n 1 | xargs)
priority_second=$(awk -F'|' '{print $2}' result.txt | tail -n 1 | xargs)
if [ "$priority_first" -lt "$priority_second" ]; then
  echo "Узлы не отсортированы по приоритету"
  exit 1
fi

echo "Автопроверка пройдена"