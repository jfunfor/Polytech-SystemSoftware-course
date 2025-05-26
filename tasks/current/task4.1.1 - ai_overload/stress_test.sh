#!/bin/bash

# CPU нагрузка: запустить 4 процесса, каждый грузит 1 ядро
for i in {1..4}; do
  bash -c 'while :; do :; done' &
  renice -n 10 -p $! >/dev/null
  prctl --name cyber_cpu_burner --pid $!
done

# RAM нагрузка: выделить 1 ГБ на процесс
for i in {1..2}; do
  python3 -c 'a = ["X" * 1024 * 1024] * 1024' &
  prctl --name cyber_ram_eater --pid $!
done

# Диск нагрузка: активная запись в файл
dd if=/dev/zero of=disk_test_file bs=1M count=1024 oflag=direct &> /dev/null &
prctl --name cyber_disk_writer --pid $!

# Сетевая нагрузка: простой сервер + пинг
nc -l -p 8888 >/dev/null &
prctl --name cyber_net_listener --pid $!
ping -i 0.2 8.8.8.8 > /dev/null &
prctl --name cyber_net_spammer --pid $!

echo "Нагрузка запущена. Чтобы остановить: killall dd ping nc python3 bash"