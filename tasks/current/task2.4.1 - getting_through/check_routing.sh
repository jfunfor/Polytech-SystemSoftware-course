#!/bin/bash

set -e

PASS=true

# Проверка маршрута
ip route | grep -q "10.0.2.0/24 via 10.0.1.1" || PASS=false
ip route | grep -q "default via 10.0.1.1" || PASS=false

# Проверка доступности Node C
ping -c 2 -W 1 10.0.2.20 > /dev/null || PASS=false

if [ "$PASS" = true ]; then
    echo "Миссия выполнена: маршруты и форвардинг настроены верно"
    exit 0
else
    echo "Ошибка: маршруты или форвардинг не настроены корректно"
    exit 1
fi