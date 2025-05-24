# Тема Маршрутизация. Задание 1. Чем дальше в сеть.

Вы — нетраннер из подпольной группировки ZeroTrace. Вам необходимо подключить изолированный терминал старой станции связи к хакерской сети через цепочку из маршрутизаторов, избегая цифровой слежки. Только грамотная настройка маршрутизации позволит пробиться сквозь глухие узлы.

## Условие задания

Настроить маршруты и включить IP-форвардинг так, чтобы трафик из изолированной сети (Node A) проходил через промежуточный маршрутизатор (Node B) и достигал внешней сети (Node C). В задании узлы представлены Docker-контейнерами.

Структура виртуальной среды:

```
Node A (10.0.1.10/24) --- (10.0.1.1) Node B (10.0.2.1) --- (10.0.2.20) Node C
```

| Устройство (контейнер) | Интерфейсы                           | Назначение                   |
| ---------------------- | ------------------------------------ | ---------------------------- |
| Node A                 | eth0: 10.0.1.10/24                   | Хост, нуждающийся в маршруте |
| Node B                 | eth0: 10.0.1.1/24, eth1: 10.0.2.1/24 | Маршрутизатор                |
| Node C                 | eth0: 10.0.2.20/24                   | Целевая сисема               |

**Техническое задание:**
1. На Node A:
	- Добавьте статический маршрут для подсети `10.0.2.0/24` через шлюз `10.0.1.1`.
2. На Node B:
	- Убедитесь, что IP-форвардинг включён (временно и в `sysctl.conf`);
	- Убедитесь, что IP-таблица маршрутов корректна (ничего не нужно добавлять вручную, если все IP настроены верно).
3. На Node A:
	- Добавьте дефолтный маршрут через `10.0.1.1`.
4. Проверка соединения:
	- Убедитесь, что после настройки проходит `ping 10.0.2.20` с Node A.
	- Для проверки правильности цепочки запретите ICMP с Node B → Node C.

## Cоветы

Для создания структуры сети необходимо развернуть 3 контейнера:
- **node-a**: конечный хост (10.0.1.10/24);
- **node-b**: маршрутизатор (10.0.1.1/24 и 10.0.2.1/24);
- **node-c**: внешняя система (10.0.2.20/24).

Связь между контейнерами должна быть организована через veth-интерфейсы и bridge-сети Docker.

Создайте простой `Dockerfile` (одинаковый для всех узлов):
```
# Dockerfile
FROM ubuntu:22.04

RUN apt update && apt install -y iproute2 iputils-ping net-tools nano

CMD ["/bin/bash"]
```

Соберите образ `netlab-node`:
```
docker build -t netlab-node .
```

Запустите контейнеры и настройте сети:
```
#!/bin/bash
set -e

# Удаление на всякий случай
docker rm -f node-a node-b node-c 2>/dev/null || true

# Создание контейнеров
docker run -dit --name node-a --privileged --network none netlab-node
docker run -dit --name node-b --privileged --network none netlab-node
docker run -dit --name node-c --privileged --network none netlab-node

# Создание сетей (bridge заменен на ручную связку через veth)
ip link add veth-a type veth peer name veth-b-a
ip link add veth-c type veth peer name veth-b-c

# Подключение интерфейсов к контейнерам
pid_a=$(docker inspect -f '{{.State.Pid}}' node-a)
pid_b=$(docker inspect -f '{{.State.Pid}}' node-b)
pid_c=$(docker inspect -f '{{.State.Pid}}' node-c)

mkdir -p /var/run/netns
ln -sf /proc/$pid_a/ns/net /var/run/netns/node-a
ln -sf /proc/$pid_b/ns/net /var/run/netns/node-b
ln -sf /proc/$pid_c/ns/net /var/run/netns/node-c

# Подключение интерфейсов
ip link set veth-a netns node-a
ip link set veth-b-a netns node-b

ip link set veth-c netns node-c
ip link set veth-b-c netns node-b

# Настройка IP-адресов
ip netns exec node-a ip addr add 10.0.1.10/24 dev veth-a
ip netns exec node-a ip link set veth-a up

ip netns exec node-b ip addr add 10.0.1.1/24 dev veth-b-a
ip netns exec node-b ip addr add 10.0.2.1/24 dev veth-b-c
ip netns exec node-b ip link set veth-b-a up
ip netns exec node-b ip link set veth-b-c up

ip netns exec node-c ip addr add 10.0.2.20/24 dev veth-c
ip netns exec node-c ip link set veth-c up

# loopback
for ns in node-a node-b node-c; do
  ip netns exec $ns ip link set lo up
done

echo "Сеть настроена. Контейнеры запущены."
```

После этого вы можете приступить к выполнению задания, заходя в каждый контейнер через `docker exec -it <node> bash`.

## Цели по теме Маршрутизация

| Тема                             | Применение                                                                          |
| -------------------------------- | ----------------------------------------------------------------------------------- |
| Просмотр таблицы маршрутизации   | Просмотр и изучение структуры таблицы маршрутизации для проверки выполнения задания |
| Добавление статического маршрута | Создание статических маршрутов согласно условию задания                             |
| Добавление дефолтного маршрута   | Добавление дефолтного маршрута (`/etc/sysctl.conf`)                                 |
| Форвардинг пакетов               | Форвардинг пакетов, проверка трассировки `traceroute` и `ping`                      |

## Автоматическая проверка 

Обратите внимание на то, что будет проверяться автоматически:

1. Проверка маршрутов.
2. Проверка `ping`.
3. Проверка форвардинга на node-b.

