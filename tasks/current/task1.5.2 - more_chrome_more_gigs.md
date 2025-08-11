# Тема Устройства хранения. Задание 2. Больше хрома.

Глубоко под улицами Найт-Сити работает подпольная лаборатория **BioChrome**, создающая импланты. Вы — сисадмин, которому поручено настроить новое хранилище для данных корпорации, объем которых стремительно растет. Хранилище должно быть масштабируемым и отказоустойчивым — нужен **LVM**.

## Условие задания

1. Подготовьте 2 физических тома (physical volume, PV) — создайте PV на `/dev/sdc` и `/dev/sdd`.
2. Создайте группу томов (volume group, VG) `biochrome_vg`.
3. Создайте 2 логических тома (logical volume, LV):
	- `data_lv` — 1GB, файловая система `xfs`;
	- `backup_lv` — 500MB, файловая система `ext4`.
4. Cмонтируйте LV:
	- `data_lv` → `/opt/data`;
	- `backup_lv` → `/mnt/backup`;
5. Настройте автоматическое монтирование через `/etc/fstab`.

**Ожидаемый результат в формате вывода команд:**

```
$ pvs ...
PV         VG           Fmt  Attr PSize PFree
/dev/sdc   biochrome_vg lvm2 a-- 
/dev/sdd   biochrome_vg lvm2 a-- 

$ ...
VG           #PV #LV #SN Attr   VSize   VFree
biochrome_vg   2   2   0 wz--n-

$ ...
LV        VG           Attr       LSize Pool Origin Data%  Meta%
data_lv   biochrome_vg -wi-ao---- 1.00g
backup_lv biochrome_vg -wi-ao---- 500.00m

$ ...
NAME                    FSTYPE LABEL UUID                                  MOUNTPOINT
...
├─biochrome_vg-data_lv   xfs         3a1b2c3d-1234-5678-9101-112131415161  /opt/data
└─biochrome_vg-backup_lv ext4        4e5f6g7h-9101-1121-3141-516171819202  /mnt/backup

$ ...
/dev/mapper/biochrome_vg-data_lv on /opt/data type xfs (rw,relatime,attr2,...)
/dev/mapper/biochrome_vg-backup_lv on /mnt/backup type ext4 (rw,relatime)
```

Это пример, у вас будут другие цифры и UUID.

## Автоматическая проверка 

Обратите внимание на то, что будет проверяться автоматически:

1. Наличие и корректность physical volumes, volume group, logical volumes.
2. Корректность файловых систем.
3. Корректность точек монтирования.
4. Корректность `/etc/fstab`.