#!/bin/bash

# 1. Проверка наличия выходного файла
if [ ! -f "filtered_logs.txt" ]; then
  echo "Файл filtered_logs.txt не найден"
  exit 1
fi

# 2. Проверка количества строк
expected_lines=3
actual_lines=$(grep -c '' filtered_logs.txt)
if [ "$actual_lines" -ne "$expected_lines" ]; then
  echo "Ожидалось $expected_lines строки(ок), найдено $actual_lines"
  exit 1
fi

# 3. Проверка формата строк
if ! grep -qE '^[0-9]{2}:[0-9]{2}:[0-9]{2} +[A-Z]+ +.+' filtered_logs.txt; then
  echo "Формат строк неверен"
  exit 1
fi

# 4. Проверка сортировки по имени модуля
if ! diff <(cut -c10-16 filtered_logs.txt | sort) <(cut -c10-16 filtered_logs.txt) >/dev/null; then
  echo "Строки не отсортированы по имени модуля"
  exit 1
fi

echo "Все проверки пройдены"