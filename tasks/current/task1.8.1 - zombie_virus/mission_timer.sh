#!/bin/bash

TIME_LIMIT=600  # 10 минут
CHECK_INTERVAL=30
UUID=$(uuidgen)
echo "$UUID" > mission_token.txt

echo "Миссия началась. У вас $((TIME_LIMIT / 60)) минут, чтобы остановить GhostForker."
echo "Таймер запущен..."

SECONDS=0
while [ $SECONDS -lt $TIME_LIMIT ]; do
    sleep $CHECK_INTERVAL
    echo "Прошло $SECONDS секунд..."
done

echo "ВРЕМЯ ИСТЕКЛО!"
echo "GhostForker распространился по сети. Задание провалено."

# Завершаем все экземпляры вируса и удаляем их PID'ы
pkill -f ghost_forker.sh
rm -f ghost.pid