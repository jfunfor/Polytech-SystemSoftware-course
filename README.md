1. Get Started: Установка Linux и подключение к виртуальной машине

Цель: Успешно установить дистрибутив Linux и настроить подключение к виртуальной машине для взаимодействия с ней.

2. Выбрать дистрибутив Linux:
- Определить свои цели, опыт и предпочтения.
- Рассмотреть популярные дистрибутивы для начинающих (Ubuntu, Linux Mint, elementary OS).
- Учесть, что Ubuntu и Linux Mint - отличные варианты для первого знакомства.
3. Установить дистрибутив Linux:
- Определить способ установки: на физическую машину (Dual Boot или полная замена) или на виртуальную машину.
- Если установка на физическую машину:
    1) Скачать ISO-образ дистрибутива с официального сайта.
    2) Создать загрузочный носитель (USB-флешку или DVD-диск) с помощью Rufus, Etcher или аналогичной программы.
    3) Перезагрузить компьютер и войти в меню загрузки BIOS/UEFI.
    4) Выбрать загрузку с созданного носителя.
    5) Следовать инструкциям установщика, особенно осторожно при Dual Boot, чтобы не удалить существующую ОС.
- Если установка на виртуальную машину:
    1) Установить программу виртуализации (VirtualBox, VMware Workstation Player/Pro).
    2) Создать новую виртуальную машину, указав имя, тип ОС (Linux), версию, объем оперативной памяти и дискового пространства.
    3) В настройках виртуальной машины указать скачанный ISO-образ в качестве загрузочного диска.
    4) Запустить виртуальную машину.
    5) Следовать инструкциям установщика Linux.
4. Настроить подключение к виртуальной машине:
- Выбрать способ подключения: через окно виртуальной машины или по SSH.
- Если подключение через окно виртуальной машины: Просто открыть окно виртуальной машины в программе виртуализации.
- Если подключение по SSH:
    1) Настроить сеть в виртуальной машине (если необходимо).
    2) Установить SSH-сервер в Linux (Ubuntu):
      sudo apt update
      sudo apt install openssh-server
      sudo systemctl enable ssh
      sudo systemctl start ssh
    3) Подключиться по SSH с основной операционной системы:
      ssh username@ip_address
    *Заменить username на имя пользователя в Linux.
    *Заменить ip_address на IP-адрес виртуальной машины (узнать с помощью ip addr в Linux).
    *Установить SSH-клиент (например, PuTTY) в Windows, если необходимо.


## Командная оболочка: Руководство по освоению

**Цель:** Научиться эффективно использовать командную оболочку Linux для навигации, исследования системы, выполнения операций с файлами и каталогами, создания команд, перенаправления ввода/вывода, использования горячих клавиш и работы с cron.

**1. Навигация:**

*   **Что сделать:** Научиться перемещаться по файловой системе.
*   **Команды:**
    *   `pwd` - Показать текущий рабочий каталог.
    *   `cd <путь>` - Перейти в указанный каталог.
        *   `cd` - Перейти в домашний каталог.
        *   `cd ..` - Перейти в родительский каталог.
        *   `cd -` - Перейти в предыдущий каталог.
        *   Использовать абсолютные и относительные пути.

**2. Исследование системы:**

*   **Что сделать:** Научиться просматривать содержимое директорий и файлов различными способами.
*   **Команды:**
    *   `ls <опции> <путь>` - Вывести список файлов и каталогов.
        *   `ls -l` - Вывести подробную информацию (права, размер, дата изменения и т.д.).
        *   `ls -a` - Показать скрытые файлы и каталоги (начинающиеся с точки).
        *   `ls -t` - Сортировать по времени изменения (последние измененные первыми).
        *   `ls -R` - Рекурсивно вывести содержимое подкаталогов.
    *   `cat <файл>` - Вывести содержимое файла на экран.
    *   `less <файл>` - Просмотреть файл постранично.
    *   `head <файл>` - Вывести первые несколько строк файла.
    *   `tail <файл>` - Вывести последние несколько строк файла.
    *   `file <файл>` - Определить тип файла.
    *   `du <опции> <путь>` - Оценить использование дискового пространства.
        *   `du -h` - Вывести размер в человекочитаемом формате.
        *   `du -s` - Вывести только суммарный размер каталога.

**3. Операции с файлами и каталогами:**

*   **Что сделать:** Научиться создавать, удалять, переименовывать, копировать и редактировать файлы и каталоги.
*   **Команды:**
    *   `mkdir <путь>` - Создать новый каталог.
        *   `mkdir -p <путь>` - Создать все необходимые родительские каталоги.
    *   `rmdir <путь>` - Удалить пустой каталог.
    *   `rm <опции> <путь>` - Удалить файлы или каталоги.
        *   `rm -i <файл>` - Запрашивать подтверждение перед удалением.
        *   `rm -r <каталог>` - Удалить каталог рекурсивно (вместе со всем содержимым!). **Осторожно!**
        *   `rm -f <файл>` - Удалить файл принудительно (без запроса подтверждения). **Осторожно!**
    *   `cp <опции> <источник> <назначение>` - Скопировать файлы или каталоги.
        *   `cp -r <каталог> <назначение>` - Скопировать каталог рекурсивно.
    *   `mv <источник> <назначение>` - Переместить или переименовать файлы или каталоги.
    *   `touch <файл>` - Создать пустой файл или изменить время последнего доступа.
    *   `nano <файл>`, `vim <файл>`, `emacs <файл>` - Открыть файл для редактирования в текстовом редакторе. (Выбрать удобный редактор).
    *   `find <путь> <опции> <действие>` - Найти файлы и каталоги, соответствующие заданным критериям.
        *   `find . -name "*.txt"` - Найти все файлы с расширением .txt в текущем каталоге и его подкаталогах.
        *   `find . -type d` - Найти все каталоги в текущем каталоге и его подкаталогах.
        *   `find . -size +1M` - Найти все файлы размером больше 1 мегабайта.
        *   `find . -mtime -7` - Найти все файлы, измененные за последние 7 дней.
        *   `find . -name "*.txt" -exec rm {} \;` - Удалить все файлы с расширением .txt (очень опасно!).

**4. Работа с командами:**

*   **Что сделать:** Научиться создавать свои команды, использовать документацию и утилиты командной строки.
*   **Команды и концепции:**
    *   `alias <имя>="команда"` - Создать псевдоним для команды.
        *   Например, `alias la="ls -la"` создаст псевдоним `la` для команды `ls -la`.
    *   Функции оболочки: создавать более сложные команды с использованием переменных и условных операторов. (Более продвинутый уровень).
    *   `man <команда>` - Открыть руководство (manual) для команды.
    *   `apropos <ключевое_слово>` - Поиск команд, связанных с ключевым словом.
    *   Использовать утилиты командной строки (например, `grep`, `sed`, `awk`, которые уже упоминались).
    *   Понимать структуру команд: `команда [опции] [аргументы]`.
    *   Различать встроенные команды оболочки и внешние исполняемые файлы.

**5. Перенаправление:**

*   **Что сделать:** Разобраться в различных способах перенаправления ввода, вывода и ошибок.
*   **Операторы:**
    *   `>` - Перенаправить стандартный вывод в файл (перезаписать содержимое).
    *   `>>` - Перенаправить стандартный вывод в файл (добавить в конец файла).
    *   `<` - Перенаправить стандартный ввод из файла.
    *   `2>` - Перенаправить стандартный вывод ошибок в файл.
    *   `&>` или `2>&1` - Перенаправить и стандартный вывод, и стандартный вывод ошибок в файл.
    *   `|` - Передать стандартный вывод одной команды на стандартный ввод другой команды (конвейер).
    *   `||` - Выполнить вторую команду только если первая завершилась с ошибкой (логическое "ИЛИ").
    *   `&&` - Выполнить вторую команду только если первая успешно завершилась (логическое "И").

**6. Особенности работы с клавиатурой:**

*   **Что сделать:** Использовать горячие клавиши для повышения эффективности работы в командной строке.
*   **Основные горячие клавиши:**
    *   `Ctrl+C` - Прервать выполняющуюся команду.
    *   `Ctrl+D` - Завершить ввод (эквивалентно нажатию Enter на пустой строке).
    *   `Ctrl+Z` - Остановить (suspend) выполняющуюся команду (можно продолжить выполнение с помощью `fg` или `bg`).
    *   `Ctrl+A` - Переместить курсор в начало строки.
    *   `Ctrl+E` - Переместить курсор в конец строки.
    *   `Ctrl+K` - Удалить текст от курсора до конца строки.
    *   `Ctrl+U` - Удалить текст от курсора до начала строки.
    *   `Ctrl+R` - Поиск в истории команд.
    *   `Tab` - Автодополнение имен файлов и команд.
    *   Стрелки вверх/вниз - Переключение по истории команд.

**7. Символические и жёсткие ссылки:**

*   **Что сделать:** Понять разницу между символическими и жёсткими ссылками и научиться их создавать.
*   **Команды:**
    *   `ln <источник> <ссылка>` - Создать жёсткую ссылку.
    *   `ln -s <источник> <ссылка>` - Создать символическую ссылку (симлинк).

*   **Различия:**
    *   **Жёсткая ссылка:** Создаёт новую запись в файловой системе, указывающую на те же данные, что и исходный файл. Нельзя создать жёсткую ссылку на каталог или на файл на другом разделе диска. При удалении исходного файла, жёсткая ссылка остаётся работоспособной.
    *   **Символическая ссылка:** Создаёт файл, содержащий путь к исходному файлу. Можно создавать на каталоги и файлы на других разделах. При удалении исходного файла, символическая ссылка становится "битой" (неработоспособной).

**8. Cron:**

*   **Что сделать:** Узнать, что такое cron, научиться его настраивать и понимать основные параметры.
*   **Описание:** Cron - это планировщик задач, позволяющий автоматически запускать команды и скрипты в заданное время или с заданным интервалом.
*   **Команды:**
    *   `crontab -e` - Редактировать crontab (файл, содержащий расписание задач).
    *   `crontab -l` - Показать содержимое crontab.
    *   `crontab -r` - Удалить crontab.

*   **Формат записи в crontab:**
    ```
    минута час день_месяца месяц день_недели команда
    ```
    *   `минута`: 0-59
    *   `час`: 0-23
    *   `день_месяца`: 1-31
    *   `месяц`: 1-12 (или названия месяцев: jan, feb, mar, apr, may, jun, jul, aug, sep, oct, nov, dec)
    *   `день_недели`: 0-6 (0 - воскресенье, 1 - понедельник, и т.д. или названия дней: sun, mon, tue, wed, thu, fri, sat)
    *   `*` - Любое значение.

*   **Примеры:**
    *   `0 0 * * * /path/to/script.sh` - Запускать скрипт каждый день в полночь.
    *   `*/5 * * * * /path/to/script.sh` - Запускать скрипт каждые 5 минут.
    *   `0 12 * * mon /path/to/script.sh` - Запускать скрипт каждый понедельник в 12:00.

**В результате:** Вы должны уверенно перемещаться по файловой системе, исследовать ее содержимое, выполнять базовые операции с файлами и каталогами, использовать команды для решения различных задач, понимать принципы перенаправления ввода/вывода, эффективно использовать горячие клавиши, создавать символические и жёсткие ссылки и автоматизировать задачи с помощью cron.
