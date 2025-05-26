#!/bin/bash

fail() { echo "[X] $1"; exit 1; }
pass() { echo "[V] $1"; }

# ICMP DROP
iptables -L INPUT -v -n | grep -q "icmp" && echo "[?] ICMP правило найдено" || fail "Нет DROP для ICMP на eth0"

# SSH DROP
iptables -L INPUT -v -n | grep -q "10.10.10.2.*dpt:22.*DROP" || fail "Нет блокировки SSH для attacker"

# TRUSTED ACCESS
iptables -L INPUT -v -n | grep -q "10.66.66.0/24.*ACCEPT" || fail "Нет разрешения для доверенной сети"

# DNAT
iptables -t nat -L PREROUTING -v -n | grep -q "dpt:2222.*192.168.99.10:22" || fail "DNAT не настроен"

# SNAT/MASQUERADE
iptables -t nat -L POSTROUTING -v -n | grep -q MASQUERADE || fail "MASQUERADE не настроен"

# FORWARD правило
iptables -L FORWARD -v -n | grep -q "dpt:22.*ACCEPT" || fail "Нет FORWARD разрешения на порт 22 дрона"

pass "Все проверки пройдены успешно!"