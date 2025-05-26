#!/bin/bash

GREEN="\e[32m"
RED="\e[31m"
RESET="\e[0m"

ok() { echo -e "${GREEN}[OK]${RESET} $1"; }
fail() { echo -e "${RED}[FAIL]${RESET} $1"; }

# 1. Проверка RAID
echo "[*] Проверка RAID /dev/md0..."

if [ -e /dev/md0 ]; then
    ok "/dev/md0 существует"
else
    fail "/dev/md0 не найден"
fi

if cat /proc/mdstat | grep -q "\[1/1\]"; then
    ok "RAID1 работает (1/1)"
else
    fail "RAID не в нормальном состоянии"
fi

# 2. Проверка файловой системы и inode
echo "[*] Проверка файловой системы RAID..."

FS_TYPE=$(blkid -o value -s TYPE /dev/md0)
if [[ "$FS_TYPE" == "ext4" ]]; then
    ok "Файловая система ext4"
else
    fail "Файловая система не ext4: $FS_TYPE"
fi

INODE_SIZE=$(sudo tune2fs -l /dev/md0 | grep "Inode size" | awk '{print $3}')
if [[ "$INODE_SIZE" -eq 256 ]]; then
    ok "Размер inode = 256 байт"
else
    fail "Размер inode = $INODE_SIZE (ожидался 256)"
fi

# 3. Проверка монтирования
echo "[*] Проверка монтирования RAID..."

if findmnt | grep -q "/mnt/ai_storage"; then
    ok "/mnt/ai_storage смонтирован"
else
    fail "/mnt/ai_storage не смонтирован"
fi

# 4. Проверка swap
echo "[*] Проверка swap..."

if swapon --show | grep -q "/swapfile"; then
    ok "Swap активен"
else
    fail "Swap НЕ активен"
fi

# 5. Проверка fstab
echo "[*] Проверка записей в /etc/fstab..."

if grep -q "/mnt/ai_storage" /etc/fstab; then
    ok "fstab содержит запись для /mnt/ai_storage"
else
    fail "fstab НЕ содержит запись для /mnt/ai_storage"
fi

if grep -q "/swapfile" /etc/fstab; then
    ok "fstab содержит запись для swap"
else
    fail "fstab НЕ содержит запись для swap"
fi

echo -e "\nПроверка завершена."