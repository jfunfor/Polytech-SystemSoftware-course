#!/bin/bash

IMAGE_NAME="postgres2077"
CONTAINER_NAME="pg2077"
PORT=5432

echo "Проверка образа..."

docker build -t $IMAGE_NAME . || {
  echo "Сборка не удалась. Владос будет разочарован.";
  exit 1;
}

echo "Запуск контейнера..."

docker rm -f $CONTAINER_NAME 2>/dev/null
docker run -d -p $PORT:5432 --name $CONTAINER_NAME $IMAGE_NAME || {
  echo "Контейнер не запустился. Где-то Alpine снова даёт сбой.";
  exit 1;
}

echo "Ожидание запуска PostgreSQL..."
sleep 5

echo "Проверка подключения к базе данных..."

docker exec -u postgres $CONTAINER_NAME psql -U postgres -c '\l' >/dev/null 2>&1 || {
  echo "Не удалось подключиться к PostgreSQL. Может, Владос опять напутал?";
  docker logs $CONTAINER_NAME
  exit 1;
}

echo "Проверка пасхалки..."

output=$(docker exec -u postgres $CONTAINER_NAME psql -U postgres -t -c 'SELECT * FROM cyberpunk_lore;' 2>/dev/null)

if [[ "$output" == *"V — всё ещё жив."* ]]; then
  echo "Всё чётко, брат. Контейнер живой, пасхалка на месте."
else
  echo "Таблица cyberpunk_lore не найдена или пасхалка пропала."
  echo "Что вернулось: $output"
  exit 1;
fi

echo "Задание выполнено. Возьми с полки пирожок!"