#!/bin/bash
set -e

# Очистка
iptables -F
iptables -t nat -F
iptables -X

# Политики
iptables -P INPUT DROP
iptables -P FORWARD DROP
iptables -P OUTPUT ACCEPT

# Loopback
iptables -A INPUT -i lo -j ACCEPT

# Разрешить уже установленные соединения
iptables -A INPUT -m conntrack --ctstate ESTABLISHED,RELATED -j ACCEPT
iptables -A FORWARD -m conntrack --ctstate ESTABLISHED,RELATED -j ACCEPT

# 1. Запрет ICMP с public_net
iptables -A INPUT -i eth0 -p icmp -j DROP

# 2. Запрет SSH для attacker
iptables -A INPUT -s 10.10.10.2 -p tcp --dport 22 -j DROP

# 3. Разрешить доверенную сеть (условно)
iptables -A INPUT -s 10.66.66.0/24 -j ACCEPT

# 4. NAT (SNAT/MASQUERADE)
iptables -t nat -A POSTROUTING -s 192.168.99.0/24 -o eth0 -j MASQUERADE

# 5. DNAT: порт 2222 перенаправляется на дрона
iptables -t nat -A PREROUTING -i eth0 -p tcp --dport 2222 -j DNAT --to-destination 192.168.99.10:22
iptables -A FORWARD -p tcp -d 192.168.99.10 --dport 22 -j ACCEPT