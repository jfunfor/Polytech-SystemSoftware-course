#!/bin/bash

if [ ! -f mission_token.txt ]; then
    echo "Таймер не был запущен. Запустите mission_timer.sh."
    exit 1
fi


# 1. Проверка, что процесс был запущен
if pgrep -f ghost_forker.sh > /dev/null; then
    echo "Процесс найден."
else
    echo "Процесс не найден."
fi

# 2. Проверка, есть ли зомби
ZOMBIE_COUNT=$(ps -eo stat,cmd | grep '^Z' | grep -v grep | wc -l)
if [ "$ZOMBIE_COUNT" -gt 0 ]; then
    echo "Найдено $ZOMBIE_COUNT зомби-процессов."
else
    echo "Зомби-процессы устранены."
fi

# 3. Проверка корректного завершения процесса
if [ -f ghost.pid ]; then
    PID=$(cat ghost.pid)
    if ps -p $PID > /dev/null; then
        echo "Процесс все еще активен. Завершите его."
    else
        echo "Процесс завершен."
    fi
else
    echo "PID файл не найден. Вы запускали start_ghost.sh?"
fi