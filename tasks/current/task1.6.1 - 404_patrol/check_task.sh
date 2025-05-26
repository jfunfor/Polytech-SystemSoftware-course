#!/bin/bash

echo "== Проверка установленных утилит =="

for pkg in curl ufw fail2ban; do
    if dpkg -l | grep -q "^ii  $pkg"; then
        echo "[OK] $pkg установлен"
    else
        echo "[FAIL] $pkg не найден"
    fi
done

echo "== Проверка отсутствия telnet =="

if dpkg -l | grep -q "^ii  telnet"; then
    echo "[FAIL] telnet установлен"
else
    echo "[OK] telnet удалён"
fi

echo "== Проверка списка пакетов =="

if [ -f /var/log/pkg-list.log ]; then
    echo "[OK] pkg-list.log найден"
else
    echo "[FAIL] pkg-list.log отсутствует"
fi

echo "== Проверка списка репозиториев =="

if [ -f /var/log/repos.log ]; then
    echo "[OK] repos.log найден"
else
    echo "[FAIL] repos.log отсутствует"
fi

echo "== Проверка настройки подписи APT =="

if grep -q 'APT::Get::AllowUnauthenticated "false";' /etc/apt/apt.conf.d/* 2>/dev/null; then
    echo "[OK] APT запрещает неподписанные пакеты"
else
    echo "[FAIL] APT разрешает неподписанные пакеты"
fi