#!/bin/bash
set -e

echo "[Проверка docker-compose инфраструктуры]"

FAIL=0

# Проверка наличия контейнеров
for c in node-a node-b node-c; do
  docker inspect "$c" >/dev/null 2>&1 || {
    echo "Контейнер $c не найден"
    FAIL=1
  }
done

# Проверка IP форвардинга
docker exec node-b sysctl net.ipv4.ip_forward | grep -q "1" || {
  echo "IP форвардинг не включён в node-b"
  FAIL=1
}

# Проверка маршрутов
docker exec node-a ip route | grep -q "10.0.2.0/24 via 10.0.1.1" || {
  echo "Маршрут до 10.0.2.0/24 не настроен в node-a"
  FAIL=1
}
docker exec node-a ip route | grep -q "default via 10.0.1.1" || {
  echo "Дефолтный маршрут не настроен в node-a"
  FAIL=1
}

# Проверка пинга
docker exec node-a ping -c 2 -W 1 10.0.2.20 > /dev/null || {
  echo "Ping с node-a до node-c (10.0.2.20) не проходит"
  FAIL=1
}

if [ "$FAIL" -eq 0 ]; then
  echo "Все проверки пройдены. Отличная работа!"
  exit 0
else
  echo "Обнаружены ошибки. Проверь настройки."
  exit 1
fi