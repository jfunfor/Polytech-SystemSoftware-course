# Тема Межсетевой экран. Задание 1. Настройка фаервола. 

Вы — сисадмин повстанческого узла связи, спрятанного в трущобах Найт-Сити. Корпорация **Arasaka** недавно начала использовать ботнет на ICMP-основе для слежки и атак, в результате чего через внешний интерфейс к вам постоянно лезет мусор.  

У вас есть шлюз-диспетчер (контейнер `firewall`), через который взаимодействуют дроны (`drone`) и возможные клиенты (`attacker`).

Ваша задача — настроить **межсетевой экран (iptables)** и **NAT**, чтобы:
1. Надёжно блокировать вредоносный трафик.
2. Разрешать доступ только проверенным союзникам.
3. Сохранять возможность выхода дронов в интернет.

## Условие задания

**Что есть**

- Контейнер `firewall` с двумя сетевыми интерфейсами:
    - `public_net` — внешняя сеть "интернет" (10.10.10.0/24);
    - `private_net` — сеть дронов (192.168.99.0/24);
- Контейнер `attacker` в `public_net` — эмулирует внешнюю угрозу.
- Контейнер `drone` в `private_net` — дрон-получатель.

**Что требуется**

На контейнере `firewall`:
1. **Запретить весь ICMP** на `public_net` (пинг и прочее).
2. **Запретить доступ к SSH (порт 22)** только для IP `10.10.10.2` (контейнер `attacker`).
3. **Разрешить** полный доступ ко всем портам только из подсети `10.66.66.0/24`.
4. Все остальное — **блокировать по умолчанию**.
5. Настроить **SNAT (MASQUERADE)** для выхода дронов в `public_net`.
6. Настроить **DNAT**: перенаправлять входящие на `10.10.10.1:2222` → `192.168.99.10:22`.

## Советы

📁 Структура задания выглядит следующим образом:
```
firewall-mission/
├── docker-compose.yml
├── Dockerfile
├── check_firewall.sh
```

📦 `docker-compose.yml`:
```
version: '3.9'

networks:
  public_net:
    driver: bridge
    ipam:
      config:
        - subnet: 10.10.10.0/24
  private_net:
    driver: bridge
    ipam:
      config:
        - subnet: 192.168.99.0/24

services:
  firewall:
    build: .
    container_name: firewall
    privileged: true
    networks:
      public_net:
        ipv4_address: 10.10.10.1
      private_net:
        ipv4_address: 192.168.99.1
    volumes:
      - ./check_firewall.sh:/check_firewall.sh
    command: sleep infinity

  attacker:
    image: debian:bookworm
    container_name: attacker
    networks:
      public_net:
        ipv4_address: 10.10.10.2
    command: sleep infinity

  drone:
    image: debian:bookworm
    container_name: drone
    networks:
      private_net:
        ipv4_address: 192.168.99.10
    command: sleep infinity
```

🐳 `Dockerfile`:
```
FROM debian:bookworm

RUN apt-get update && \
    apt-get install -y iproute2 iptables iputils-ping net-tools curl openssh-server && \
    apt-get clean

RUN echo "root:root" | chpasswd
RUN mkdir /var/run/sshd

CMD ["/usr/sbin/sshd", "-D"]
```

Для запуска всех контейнеров выполните
```
docker-compose up --build -d
```

После чего зайдите в контейнер `firewall` и настройте межсетевой экран согласно условию задания:
```
docker exec -it firewall bash
```

По окончании выполнения задания запустите автоматическую проверку:
```
docker exec -it firewall bash /check_firewall.sh
```

## Цели по теме Межсетевой экран

| Тема                                        | Применение                                                              |
| ------------------------------------------- | ----------------------------------------------------------------------- |
| iptables, основные цепочки, методы, правила | Изучение правил и работы с iptables для настройки своего фаервола       |
| Запрет ICMP протокола, портов               | Запрет ICMP для внешней сети, запрет порта 22 для конкретного IP-адреса |
| Настройка NAT                               | Настройка DNAT согласно условию задания                                 |

## Автоматическая проверка

Автоматически проверяется правильность выполнения каждого без исключений пункта из раздела "Что требуется".