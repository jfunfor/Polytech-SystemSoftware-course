#!/bin/bash

docker compose up -d --build
sleep 5

docker exec client bash ping_gosu.sh > result.txt

if grep -q "GOSU-PASS-" result.txt; then
    echo "[+] Задание выполнено успешно!"
    exit 0
else
    echo "[-] Ошибка: токен не получен."
    exit 1
fi