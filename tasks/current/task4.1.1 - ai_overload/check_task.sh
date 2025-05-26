#!/bin/bash

check_file_contains() {
  if grep -qi "$2" "$1"; then
    echo "Найдено '$2' в $1"
    return 0
  else
    echo "НЕ найдено '$2' в $1"
    return 1
  fi
}

score=0

# Проверка наличия файлов
for f in cpu.log ram.log disk.log dmesg_filtered.log network.log report.md; do
  if [ -f "$f" ]; then
    ((score++))
  fi
done

# Проверка ключевых процессов
check_file_contains cpu.log "cyber_cpu_burner" && ((score++))
check_file_contains ram.log "cyber_ram_eater" && ((score++))
check_file_contains disk.log "disk_test_file" && ((score++))
check_file_contains network.log "8888" && ((score++))
check_file_contains network.log "cyber_net_spammer" && ((score++))

# Итог
echo "Итого: $score баллов из 10"