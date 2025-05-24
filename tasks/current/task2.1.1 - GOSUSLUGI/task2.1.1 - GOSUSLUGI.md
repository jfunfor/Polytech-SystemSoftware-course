# Тема Протоколы сетевого уровня. Задание 1. Как получить загранпаспорт?

В 2077 году миром правит цифровая бюрократия. Доступ к порталу **gosuslugi.net** охраняется суровой цепочкой роутеров. Один из них — легендарный **роутер "Батя"**, настроенный в далеких 90-х.

С портала госуслуг вам необходимо получить код активации загранпаспорта нового поколения. Только вот незадача: вам придется пробить три маршрутизатора, на последнем из которых закрыты все порты, кроме **ICMP**, а сервер госуслуг ответит на ICMP Echo-запрос **только если в payload будет слово "пж"**.

## Условие задания

Вы получаете готовый `docker-compose.yml` с 4 контейнерами:
- `client` — ваша машина;
- `router1` — роутер с IP 10.10.10.1;
- `router2` — роутер с IP 10.20.20.1;
- `router_batya` — суровый Linux-роутер, где `iptables` всё блочит, кроме ICMP;
- `gosuslugi` — конечная цель с IP 10.30.30.10, отвечает на ICMP с payload "пж".

**Что нужно сделать:**
- Настроить маршруты на клиенте, чтобы ICMP дошёл до `gosuslugi` через все роутеры.
- Отправить **ICMP Echo-запрос** со специальным payload: `пж`.
- Получить ICMP Echo-reply с флагом: `GOSU-PASS-[...код...]`.

Задание оформляется в виде скрипта `ping_gosu.sh`, который должен:
- настроить маршруты (через `ip route add`);
- послать ICMP с нужным payload (можно `hping3`, `ping -p`, `scapy`, `python`);
- распарсить ответ и вывести флаг.

## Советы

📁 Структура задания выглядит следующим образом:
```
project/
├── client/
│   └── ping_gosu.sh
├── router1/
│   └── Dockerfile
├── router2/
│   └── Dockerfile
├── router_batya/
│   └── Dockerfile
├── gosuslugi/
│   └── ping_responder.py
├── docker-compose.yml
├── verify.sh
└── README.md
```

📦 `docker-compose.yml` — создает 5 контейнеров и подключает их в нужной последовательности через `networks`:
```
version: '3.9'

services:
  client:
    build: ./client
    container_name: client
    networks:
      net1:
        ipv4_address: 10.10.10.2
    cap_add:
      - NET_ADMIN
    tty: true

  router1:
    build: ./router
    container_name: router1
    networks:
      net1:
        ipv4_address: 10.10.10.1
      net2:
        ipv4_address: 10.20.20.1
    cap_add:
      - NET_ADMIN
    tty: true

  router2:
    build: ./router
    container_name: router2
    networks:
      net2:
        ipv4_address: 10.20.20.2
      net3:
        ipv4_address: 10.30.30.1
    cap_add:
      - NET_ADMIN
    tty: true

  router_batya:
    build: ./router_batya
    container_name: router_batya
    networks:
      net3:
        ipv4_address: 10.30.30.2
      net4:
        ipv4_address: 10.40.40.1
    cap_add:
      - NET_ADMIN
    tty: true

  gosuslugi:
    build: ./gosuslugi
    container_name: gosuslugi
    networks:
      net4:
        ipv4_address: 10.40.40.10
    cap_add:
      - NET_ADMIN
    tty: true

networks:
  net1:
    driver: bridge
    ipam:
      config:
        - subnet: 10.10.10.0/24
  net2:
    driver: bridge
    ipam:
      config:
        - subnet: 10.20.20.0/24
  net3:
    driver: bridge
    ipam:
      config:
        - subnet: 10.30.30.0/24
  net4:
    driver: bridge
    ipam:
      config:
        - subnet: 10.40.40.0/24
```

🐳 `client/Dockerfile`:
```
FROM ubuntu:20.04
RUN apt update && apt install -y iproute2 iputils-ping hping3 net-tools curl
WORKDIR /root
COPY ping_gosu.sh .
RUN chmod +x ping_gosu.sh
CMD ["bash"]
```

🐳 `router/Dockerfile`:
```
FROM ubuntu:20.04
RUN apt update && apt install -y iproute2 iptables
RUN sysctl -w net.ipv4.ip_forward=1
CMD ["bash"]
```

🐳 `router_batya/Dockerfile` — роутер с блоками:
```
FROM ubuntu:20.04
RUN apt update && apt install -y iproute2 iptables

# Включаем маршрутизацию
RUN echo "net.ipv4.ip_forward=1" >> /etc/sysctl.conf

# Скрипт запуска
COPY entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh
CMD ["/entrypoint.sh"]
```

🧩 `router_batya/entrypoint.sh`:
```
#!/bin/bash

# минус вайб и IP forwarding
sysctl -w net.ipv4.ip_forward=1

# минус все, кроме ICMP
iptables -P FORWARD DROP
iptables -A FORWARD -p icmp -j ACCEPT

# Ахах не
iptables -A FORWARD -p tcp -j DROP
iptables -A FORWARD -p udp -j DROP

echo "[ROUTER BATYA] Удачи родной..."

bash
```

🐳 `gosuslugi/Dockerfile`:
```
FROM python:3.9-slim
COPY ping_responder.py /ping_responder.py
CMD ["python", "/ping_responder.py"]
```

🧠 `gosuslugi/ping_responder.py` (TODO: реализовать генерацию уникального кода, возможно, привязку к времени)
```
from scapy.all import *
import os

def handle_icmp(pkt):
    if ICMP in pkt and pkt[ICMP].type == 8:
        if b"пж" in bytes(pkt[Raw].load):
            ip = pkt[IP].src
            print(f"[+] Получен вежливый пинг от {ip}")
            reply = IP(dst=ip)/ICMP(type=0)/b"GOSU-PASS-4421"
            send(reply, verbose=0)

sniff(filter="icmp", prn=handle_icmp)
```

## Цели по теме Протоколы сетевого уровня

| Тема          | Применение                                                                        |
| ------------- | --------------------------------------------------------------------------------- |
| IP-адресация  | Настройка маршрутов на клиенте (`ip route`)                                       |
| ICMP протокол | Знакомство с протоколом и форматом пакета ICMP, инкапсуляция своих данных в пакет |
| Команда ping  | Отправка пинга (ICMP с кастомным payload)                                         |

## Автоматическая проверка

Скрипт автоматической проверки запускает написанный вами скрипт и проверяет получение корректного ответа от госуслуг.


