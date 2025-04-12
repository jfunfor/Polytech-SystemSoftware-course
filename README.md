1.  УСТРОЙСТВА ХРАНЕНИЯ

Устройства хранения данных являются критически важными компонентами любой вычислительной системы. В контексте операционной системы Linux они служат для хранения операционной системы, приложений и пользовательских данных. Устройства хранения можно классифицировать на несколько типов:

- Постоянные устройства: используются для долговременного хранения данных. К ним относятся жесткие диски (HDD) и твердотельные накопители (SSD).
- Временные устройства: предоставляют временное хранилище, как, например, RAM-диски, которые используют оперативную память для хранения данных.
- Сетевые устройства: к ним относятся NAS (Network Attached Storage) и SAN (Storage Area Network), которые обеспечивают доступ к данным через сеть.
  
Почему это важно в Linux?
В Linux все устройства представлены как файлы, а диски и их разделы активно используются при установке, управлении системой, резервном копировании, расширении хранилища и других задачах.


2. Файловые системы: типы и преимущества
   
Файловая система — это способ организации и хранения данных на диске. Она отвечает за то, как файлы записываются, читаются и управляются.
- EXT4
Наиболее популярная и стабильная файловая система.
Преимущества:
Поддерживает журналирование — записи об изменениях сначала сохраняются в журнал, что обеспечивает целостность данных.
Поддерживает тома до 1 эксабайта и файлы до 16 ТБ.
Быстрое восстановление после сбоев.
Высокая производительность и стабильность.
- XFS
Высокопроизводительная файловая система, идеально подходит для работы с большими файлами.
Преимущества:
Отличается высокой скоростью параллельных операций.
Поддерживает онлайн-расширение томов (но не уменьшение).
Недостаток: нет возможности уменьшить размер раздела.
- Btrfs
Современная, модульная файловая система, включающая в себя функции управления томами, которые обычно реализуются через LVM или RAID.
Преимущества:
Поддерживает сжатие, снапшоты, дедупликацию, встроенные RAID-механизмы(0/1/10).
Идеальна для сложных систем и тех, где важна управляемость и гибкость.
Недостатки:
Более высокая нагрузка на процессор.
Меньшая стабильность по сравнению с EXT4.
- F2FS
Разработана специально для флеш-накопителей.
Эффективно работает с eMMC и SSD, продлевая срок их службы.
Преимущества:
Быстрая запись и чтение.
Оптимизированная работа с фрагментированными данными.
Эффективное использование памяти и I/O.
- NTFS, VFAT
Используются в основном для совместимости с Windows.
VFAT (или FAT32) не поддерживает права и журналирование, но широко совместима. Bдеально подходит для флешек и кросс-платформенного обмена.
NTFS - продвинутая файловая система Windows, в Linux поддерживается через ntfs-3g — можно читать и записывать, но с некоторыми ограничениями.


3. Разметка дисков и монтирование директорий
Перед использованием устройства его нужно разметить и создать на нём файловую систему. Подразумевает создание логических разделов на физическом диске. Обычно создаётся одна таблица разделов (MBR или GPT), и далее в ней — несколько разделов.
Разметка дисков:
fdisk — классический инструмент для работы с MBR.
gdisk — для GPT-разметки.
parted/gparted — более гибкие, поддерживают работу с большими дисками.

Шаги для разметки и монтирования директорий:
1. Определение диска
Посмотреть список подключённых дисков:
lsblk или: sudo fdisk -l
Допустим, диск — это /dev/sdb.

2. Создание раздела
Вариант A: с помощью fdisk (для MBR-разметки)
sudo fdisk /dev/sdb
Далее в интерактивном режиме:
n — создать новый раздел
p — основной (primary)
1 — номер раздела
нажать Enter для значения по умолчанию (начало/конец)
w — записать изменения
Вариант B: с помощью parted (для GPT и больших дисков)
sudo parted /dev/sdb
Команды внутри:
mklabel gpt - создаём GPT таблицу разделов
mkpart primary ext4 0% 100%
quit

3. Создание файловой системы
После создания раздела, например /dev/sdb1, нужно создать на нём файловую систему:
sudo mkfs.ext4 /dev/sdb1 (Вместо ext4 можно использовать другие: xfs, btrfs и т.д.)

4. Монтирование директории
Теперь, когда раздел готов, его можно смонтировать — подключить к определённой директории в файловой системе.
4.1. Создание точки монтирования
sudo mkdir /mnt/mydisk

4.2. Монтирование вручную
sudo mount /dev/sdb1 /mnt/mydisk
Теперь содержимое /dev/sdb1 доступно в /mnt/mydisk.

5. Проверка
sudo mount -a
Если ошибок нет — всё настроено корректно.


4. LVM — гибкое управление томами
Что такое LVM
LVM (Logical Volume Manager) — это система логического управления разделами. Она позволяет объединять несколько физических дисков в одно логическое пространство, а также гибко изменять размеры томов без потери данных.
Зачем использовать LVM? 
Возможность увеличивать и уменьшать разделы "на лету".
Объединение нескольких физических устройств.
Создание снапшотов — снимков текущего состояния данных.
Простота резервного копирования и масштабирования.
Основные компоненты LVM:
PV (Physical Volume) — физический том (обычно раздел).
VG (Volume Group) — группа томов, объединяющая PV.
LV (Logical Volume) — логический том, на котором создаётся файловая система.

Основные настройки и команды:
Создать PV - pvcreate /dev/sdb1. Посмотреть PV - pvs или pvdisplay. Создать VG - vgcreate my_vg /dev/sdb1. Расширить VG - vgextend my_vg /dev/sdc1. Посмотреть VG - vgs или vgdisplay. Создать LV - lvcreate -L 5G -n my_lv my_vg. Увеличить LV - lvextend -L +5G /dev/my_vg/my_lv. Уменьшить LV - (сначала umount, затем resize2fs, lvreduce). Посмотреть LV - lvs или lvdisplay.


5. RAID - массивы (Redundant Array of Independent Disks)
Raid это технология, которая используется для объединения нескольких физических дисков в один логический массив с целью повышения производительности, надежности или их сочетания. В Linux поддержка RAID осуществляется через утилиту mdadm (для программного RAID) и аппаратные RAID-контроллеры.
RAID позволяет:
- Увеличить производительность — с помощью параллельной записи и чтения данных с нескольких дисков.
- Обеспечить избыточность данных — с помощью дублирования данных для защиты от потерь.
- Оптимизировать использование дискового пространства — комбинируя несколько дисков в один большой логический том.
Различные уровни RAID:
- RAID 0: делит данные между дисками для повышения скорости, но не обеспечивает надежности. Идеально для задач с высокой производительностью.
- RAID 1: зеркалирует данные на два диска для повышения надежности. Подходит для критически важных данных.
- RAID 5: использует чередование и паритет для обеспечения надежности и скорости. Хорошо подходит для серверов и хранения больших объемов данных с требованием надежности.
- RAID 10: комбинирует RAID 0 и RAID 1 для достижения высокой производительности и надежности. Подходит для высоконагруженных серверов.
Настройка программного RAID в Linux: используется mdadm для создания и управления RAID-массивами:
mdadm --create --verbose /dev/md0 --level=5 --raid-devices=3 /dev/sdb1 /dev/sdc1 /dev/sdd1


6. Swap.
   
Swap — это область на диске, используемая как "виртуальная оперативная память".
Зачем нужен swap?
- Поддержка работы при нехватке памяти.
Когда оперативная память (RAM) заполнена, ядро может временно переместить неактивные данные в swap, чтобы освободить место для более важных задач.
- Гибернация (hibernation).
При переходе системы в спящий режим (hibernate), содержимое RAM записывается в swap, чтобы потом восстановить сессию.
- Защита от краха.
Даже если RAM полностью исчерпана, наличие swap позволяет системе работать, пусть и медленнее, вместо внезапного завершения процессов.

Виды swap:
- Swap-раздел (partition) — отдельный раздел диска, например /dev/sda2.
- Swap-файл (file) — обычный файл, который настроен как swap, например /swapfile.

7. Inode.
Каждый файл в Linux представлен структурой inode — это уникальный идентификатор и хранилище метаданных.
Что хранит inode:
- Размер файла
- Время создания, изменения, доступа
- Права доступа
- Владелец и группа
- Ссылки на блоки с данными
Имя файла хранится в каталоге, а не в inode. Это позволяет использовать жёсткие ссылки.
Ограничения inode:
Каждое хранилище (например, файловая система) имеет ограничение на количество inode. Если на диске заканчиваются inode, невозможно создать новые файлы, даже если на диске ещё есть свободное место.








2.  СЕТИ. TCP, UDP

Протокол – это набор правил, определяющий, как узлы сети должны передавать данные друг другу. Так сложилось, что есть две модели, описывающие уровни протоколов. Одна из них теоретическая – модель OSI, а другая практическая – TCP/IP.
Транспортный уровень модели TCP/IP основан на двух китах: Transmission Control Protocol и User Datagram Protocol. Есть и другие протоколы на этом уровне (например, QUICK), но они не так часто используются.
Протоколы транспортного уровня используются для адресации пакетов с порта приложения отправителя на порт приложения получателя. Более того, протоколы этого уровня не знают ничего про различия в узлах сетей. Всё, что им требуется знать про адресацию, это то, что есть приложение, отсылающее сообщение, и оно для отправки использует какой-то порт. И приложение, которое получает сообщение, тоже использует какой-то порт. Основная «адресация внутри Интернета» же реализована на межсетевом уровне.

1. UDP — это протокол, обеспечивающий ненадежную, безоценочную передачу данных. Он не гарантирует доставку, порядок или целостность пакетов, позволяя передавать данные быстрее, но с риском их потери. Лучше всего подходит для приложений, где скорость важнее надежности, таких как потоковое видео, VoIP(аудиозвонки) и онлайн-игры. UDP не требует установки соединения. Данные могут быть отправлены сразу, без предварительных шагов, что делает его более быстрым, но менее надежным.
   
UDP называется ненадежным, потому что:
- Не обеспечивает подтверждений доставки.
- Не гарантирует порядок доставки пакетов.
- Не имеет механизма повторной передачи потерянных пакетов.
  
Пакеты, передаваемые с помощью UDP, иногда называются датаграммами. Этот термин обычно используется тогда, когда важно подчеркнуть, что пакет передаётся без установки соединения.
Заголовок UDP-пакета состоит из 8 байт, которые включают в себя:
•	порт отправителя;
•	порт получателя;
•	длину датаграммы;
•	контрольную сумму.

Также стоит отметить, что потерянные пакеты не отслеживаются, и приложение, использующее UDP, должно самостоятельно обрабатывать потери, если это необходимо.

2. TCP — это протокол, обеспечивающий надежную, ориентированную на соединение передачу данных. Он гарантирует, что данные будут доставлены в правильном порядке и без потерь. Идеально подходит для приложений, требующих надежности, таких как веб-серфинг, электронная почта и передача файлов. Если UDP простой и не требующий установки соединения, то TCP – сложный и требующий. Перед тем, как начать передачу данных, клиент и сервер должны подключиться друг к другу и договориться о том, как они будут осуществлять эту самую передачу.
   
В отличие от датаграмм в UDP, пакеты, создаваемые TCP, называются сегментами.
Сегмент TCP — это блок данных, который включает заголовок TCP и полезную нагрузку. Он формируется следующим образом:
-  Заголовок TCP добавляется к данным приложения.
-  Заголовок содержит необходимую информацию для управления соединением.
-  Сегмент передается по сети к получателю, где он разбирается и передается на уровень приложения.
   
TCP называется надежным протоколом, потому что:
- Он обеспечивает проверку целостности данных с использованием контрольных сумм.
- Гарантирует доставку данных с помощью механизма подтверждений (ACK) и повторных передач.
- Управляет порядком пакетов, обеспечивая, чтобы они были собраны в правильном порядке.
  
Работы много, а потому и заголовок у TCP как минимум 20 байт, в некоторых случаях может быть больше. Он включает в себя:
-	порт отправителя;
-	порт получателя;
-	порядковый номер;
-	номер подтверждения;
-	длину заголовка;
-	флаги;
-	размер окна;
-	контрольную сумму;
-	указатель важности;
-	дополнительные опции.
  
TCP использует трехступенчатый процесс установки соединения, называемый "three-way handshake":
  -  Клиент отправляет запрос на соединение (SYN).
  -  Сервер отвечает с подтверждением (SYN-ACK).
  -  Клиент отправляет окончательное подтверждение (ACK), после чего соединение устанавливается.

Гарантия доставки в TCP обеспечивается через:
- Подтверждения (ACK): отправитель ждет подтверждения от получателя для каждого отправленного сегмента.
- Тайм-ауты: если подтверждение не получено в течение определенного времени, отправитель повторно отправляет сегмент.
- Нумерация сегментов: каждый сегмент имеет порядковый номер, что позволяет получателю собирать данные в правильном порядке.
  
Потерянные пакеты обнаруживаются с помощью подтверждений и тайм-аутов. Если ACK не получено, отправитель повторно отправляет пакет.
Для того чтобы оптимизировать использование сети и минимизировать потери пакетов, задержек и перегрузок работает Контроль перегрузок: 
Контроль перегрузки (congestion control) в TCP — это набор механизмов, предназначенных для предотвращения перегрузки сети и обеспечения эффективной передачи данных. Контроль перегрузки в TCP включает несколько алгоритмов, например, TCP Tahoe и TCP Reno. Они анализируют состояние сети и регулируют скорость передачи данных, чтобы избежать перегрузки. Если происходит потеря пакетов, скорость передачи снижается, чтобы уменьшить нагрузку на сеть.

Основные принципы контроля перегрузки
-  Измерение состояния сети: TCP использует различные методы для оценки состояния сети, включая задержку и потерю пакетов. Это позволяет определить, насколько сеть загружена в текущий момент.
-  Регулировка скорости передачи: на основе измерений состояния сети TCP регулирует скорость передачи данных. Если сеть перегружена (например, наблюдаются потери пакетов), TCP снижает скорость отправки данных.

Алгоритмы контроля перегрузки:
- 1.TCP Tahoe:
     - Использует алгоритм "Slow Start" для постепенного увеличения скорости передачи данных.
     - При обнаружении потери пакета (например, по отсутствию подтверждения) скорость сбрасывается до начального уровня.
- 2.TCP Reno:
     - Вводит механизм "Fast Recovery", который позволяет продолжать передачу данных после потери, не сбрасывая скорость до начального уровня.
     - Если теряется пакет, TCP Reno уменьшает размер окна, но не сбрасывает его полностью, что позволяет оставаться в режиме передачи данных.

Механизм скользящего окна – что это?
Скользящее окно — это механизм, который позволяет отправлять несколько сегментов данных без ожидания подтверждения каждого из них. Это увеличивает эффективность передачи данных, устанавливая максимальный размер окна, который определяет, сколько сегментов может быть отправлено до получения подтверждения.

Механизм повторной передачи (retransmission) в TCP
Если сегмент данных не был подтвержден в установленный тайм-аут, TCP повторно отправляет этот сегмент. Это обеспечивает надежность, так как потерянные данные могут быть восстановлены. Повторная передача происходит до тех пор, пока не будет получено подтверждение от получателя.

