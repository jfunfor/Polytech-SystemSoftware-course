#!/bin/bash

# Цвета
GREEN="\e[32m"
RED="\e[31m"
RESET="\e[0m"

ok() { echo -e "${GREEN}[OK]${RESET} $1"; }
fail() { echo -e "${RED}[FAIL]${RESET} $1"; }

# 1. Проверка разделов
echo "[*] Проверка разметки диска /dev/sdb..."

check_partition() {
    local part="$1"
    if lsblk /dev/sdb | grep -q "$part"; then
        ok "Раздел $part найден"
    else
        fail "Раздел $part не найден"
    fi
}

check_partition "sdb1"
check_partition "sdb2"
check_partition "sdb3"

# 2. Проверка файловых систем
echo "[*] Проверка файловых систем..."

check_fs() {
    local device="$1"
    local expected="$2"
    local actual=$(blkid -o value -s TYPE "$device")
    if [[ "$actual" == "$expected" ]]; then
        ok "$device имеет файловую систему $expected"
    else
        fail "$device имеет файловую систему $actual, ожидалась $expected"
    fi
}

check_fs "/dev/sdb1" "ext2"
check_fs "/dev/sdb3" "ext4"

# 3. Проверка swap
echo "[*] Проверка swap..."
if swapon --show | grep -q "/dev/sdb2"; then
    ok "Swap активен на /dev/sdb2"
else
    fail "Swap не активен на /dev/sdb2"
fi

# 4. Проверка монтирования
echo "[*] Проверка монтирования..."

check_mount() {
    local mountpoint="$1"
    if findmnt | grep -q "$mountpoint"; then
        ok "$mountpoint смонтирован"
    else
        fail "$mountpoint не смонтирован"
    fi
}

check_mount "/mnt/arasaka"
check_mount "/mnt/arasaka/boot"

# 5. Проверка fstab
echo "[*] Проверка записей в /etc/fstab..."

check_fstab_contains() {
    local pattern="$1"
    if grep -q "$pattern" /etc/fstab; then
        ok "fstab содержит запись: $pattern"
    else
        fail "fstab НЕ содержит запись: $pattern"
    fi
}

check_fstab_contains "/mnt/arasaka"
check_fstab_contains "/mnt/arasaka/boot"
check_fstab_contains "swap"

echo -e "\nПроверка завершена."