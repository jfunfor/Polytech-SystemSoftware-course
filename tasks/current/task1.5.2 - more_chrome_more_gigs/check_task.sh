#!/bin/bash

GREEN="\e[32m"
RED="\e[31m"
RESET="\e[0m"

ok() { echo -e "${GREEN}[OK]${RESET} $1"; }
fail() { echo -e "${RED}[FAIL]${RESET} $1"; }

# Проверка наличия PV
echo "[*] Проверка физических томов..."
for dev in /dev/sdc /dev/sdd; do
    if pvs | grep -q "$dev"; then
        ok "PV найден: $dev"
    else
        fail "PV не найден: $dev"
    fi
done

# Проверка Volume Group
echo "[*] Проверка volume group..."
if vgs | grep -q "biochrome_vg"; then
    ok "Volume group 'biochrome_vg' существует"
else
    fail "Volume group 'biochrome_vg' не найдена"
fi

# Проверка Logical Volumes
echo "[*] Проверка logical volumes..."
check_lv() {
    local lvname="$1"
    local expected_fs="$2"
    local mountpoint="$3"
    local devpath="/dev/biochrome_vg/$lvname"

    if lvs | grep -q "$lvname"; then
        ok "LV $lvname существует"
    else
        fail "LV $lvname не найден"
    fi

    # Проверка файловой системы
    fs=$(blkid -o value -s TYPE "$devpath")
    if [[ "$fs" == "$expected_fs" ]]; then
        ok "$lvname использует файловую систему $expected_fs"
    else
        fail "$lvname использует файловую систему $fs, ожидалась $expected_fs"
    fi

    # Проверка точки монтирования
    if mount | grep -q "$mountpoint"; then
        ok "$lvname смонтирован в $mountpoint"
    else
        fail "$lvname не смонтирован в $mountpoint"
    fi

    # Проверка fstab
    if grep -q "$lvname" /etc/fstab; then
        ok "fstab содержит запись для $lvname"
    else
        fail "fstab НЕ содержит запись для $lvname"
    fi
}

check_lv "data_lv" "xfs" "/opt/data"
check_lv "backup_lv" "ext4" "/mnt/backup"

echo -e "\nПроверка завершена."