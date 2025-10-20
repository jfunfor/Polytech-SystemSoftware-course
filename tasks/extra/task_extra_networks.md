## Диверсия через друга

Ваш друг Луми заражен Терминальной Лихорадкой. Вирус использует его привилегированный доступ для блокировки легальных способов повышения рейтинга. Система мониторинга обнаружила аномалию: Луми тайно манипулирует пакетами данных, искажая результаты социальной активности.

Но у нас есть шанс! Аналитики обнаружили обходной путь, который еще не контролируется вирусом. Луми знает о нем, но не может себе помочь, так как все действия критичны для работы инфраструктуры.

## Условие задачи

### Создание `docker-compose.yml`

Ниже приведен пример файла docker compose с ключевыми нюансами задачи:

```
services:
  citizen:
    image: ubuntu
    container_name: citizen
    networks:
      net1:
        ipv4_address: 10.0.0.2  # Клиент имеет статический IP для удобства

  gate1:
    image: ubuntu
    container_name: gate1
    networks:
      net1: {}  # Динамический IP
      net2: {}  # Динамический IP
    mac_address: "00:AA:BB:CC:DD:11"

  gate2:
    image: ubuntu
    container_name: gate2
    networks:
      net2: {}  # Динамический IP
      net3: {}  # Динамический IP
    mac_address: "00:AA:BB:CC:DD:22"

  gate_fate:
    image: ubuntu
    container_name: gate_fate
    networks:
      net3: {}  # Динамический IP
      net4: {}  # Динамический IP
    mac_address: "00:AA:BB:CC:DD:33"

  rating_server:
    image: ubuntu
    container_name: rating_server
    networks:
      net4:
        ipv4_address: 10.30.30.10  # Сервер имеет статический IP

networks:
  net1:
    driver: bridge
    ipam:
      config:
        - subnet: 10.0.0.0/24
  net2:
    driver: bridge
    ipam:
      config:
        - subnet: 10.10.10.0/24
  net3:
    driver: bridge
    ipam:
      config:
        - subnet: 10.20.20.0/24
  net4:
    driver: bridge
    ipam:
      config:
        - subnet: 10.30.30.0/24
```

### Написание скрипта

Создайте скрипт `credit_hack.sh`, который будет пошагово выполнять следующие задачи:

- Определить текущие IP шлюзов через ARP-сканирование (например, `arp-scan`), используя их MAC-адреса.
- Настроить маршруты так, чтобы трафик шёл через все три шлюза (используя обнаруженные IP).
- Сгенерировать токен для payload:

    > Исходная строка: `Рейтинг: 75` \
    > Шифр: XOR с ключом 0xCC \
    > Пример: `Рейтинг: 75` → hex → XOR → base64

- Отправить ICMP-запрос с параметрами:

    > TTL=7; \
    > ID=0xDEAD; \
    > Payload = зашифрованный токен. \

- Расшифровать ответ, где флаг закодирован в base64 + XOR. Ответ вывести в стандартный поток вывода. \
Формат овета: `ССР-РЕЙТИНГ: N`, где `N` - новый рейтинг

## Технические детали

- `citizen` — ваша машина (IP: динамический, интерфейс `eth0`).

- `gate1`, `gate2`, `gate_fate` — шлюзы с динамическими IP (MAC-адреса фиксированы: `00:AA:BB:CC:DD:11`, `00:AA:BB:CC:DD:22`, `00:AA:BB:CC:DD:33`).

- `rating_server` — сервер ССР (IP: `10.30.30.10`, отвечает только на ICMP с зашифрованным payload).
