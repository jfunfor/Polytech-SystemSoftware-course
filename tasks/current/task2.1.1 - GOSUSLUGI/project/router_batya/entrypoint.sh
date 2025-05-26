#!/bin/bash

# Включаем IP forwarding
sysctl -w net.ipv4.ip_forward=1

# Блокируем всё, кроме ICMP
iptables -P FORWARD DROP
iptables -A FORWARD -p icmp -j ACCEPT

# Никаких TCP/UDP
iptables -A FORWARD -p tcp -j DROP
iptables -A FORWARD -p udp -j DROP

# Логгируем для прикола
echo "[ROUTER BATYA] Всё кроме ICMP заблочено, удачи..."

bash