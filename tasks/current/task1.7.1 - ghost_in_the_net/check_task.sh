#!/bin/bash

echo "=== Проверка mission.sh ==="

# 1. Проверка на наличие ключевых команд
required_cmds=("ip a" "ip route" "cat /etc/resolv.conf" "ping" "nc" "nmap" "tcpdump")
missing=0
for cmd in "${required_cmds[@]}"; do
  if ! grep -q "$cmd" mission.sh; then
    echo "Не найдена команда: $cmd"
    missing=$((missing+1))
  else
    echo "Найдена команда: $cmd"
  fi
done

# 2. Проверка, существует ли дамп пакетов
if [ -f "ghost_trace.pcap" ] && [ -s "ghost_trace.pcap" ]; then
  echo "Файл ghost_trace.pcap существует и не пустой"
else
  echo "Проблема с файлом ghost_trace.pcap"
  missing=$((missing+1))
fi

# 3. Проверка, есть ли результат сканирования
if grep -q "open" scan_result.txt; then
  echo "Найдены открытые порты в scan_result.txt"
else
  echo "В scan_result.txt не найдены открытые порты"
  missing=$((missing+1))
fi

# Итог
if [ "$missing" -eq 0 ]; then
  echo -e "\nВсё отлично! Задание выполнено успешно."
else
  echo -e "\nОбнаружено $missing проблем(ы)."
fi