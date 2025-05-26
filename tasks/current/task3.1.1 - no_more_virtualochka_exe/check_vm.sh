#!/bin/bash

# Проверка наличия Dockerfile
if [[ ! -f Dockerfile ]]; then
  echo "Нет Dockerfile"
  exit 1
fi

# Проверка сборки образа
docker build -t chromocity/anti_virt:v1 . > /dev/null 2>&1
if [[ $? -ne 0 ]]; then
  echo "Ошибка сборки Docker-образа"
  exit 1
fi

# Удаление предыдущего контейнера, если он был
docker rm -f vasyabot > /dev/null 2>&1

# Запуск контейнера
docker run -d --name vasyabot chromocity/anti_virt:v1 > /dev/null

# Проверка наличия файла /cyber_info.txt в контейнере
docker exec vasyabot test -f /cyber_info.txt
if [[ $? -ne 0 ]]; then
  echo "Файл /cyber_info.txt не найден в контейнере"
  exit 1
fi

# Проверка содержимого лога
docker logs vasyabot > output.log

if grep -q "Да здравствует контейнеризация!" output.log && \
   grep -q "Контейнеры — это не шутки, это стиль жизни." output.log; then
  echo "Всё ок! Контейнер работает правильно."
  exit 0
else
  echo "В output.log не найден нужный текст"
  exit 1
fi