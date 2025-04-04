Темы на 1 семестр:
### Основы линукс
1. Get started
    * В результате освоения темы нужно установить дистрибутив и подключиться к виртуальной машине
2. Командная оболочка (на знание базовых команд)
    * Навигация - научиться перемещаться по директориям
    * Исследование системы - научиться смотреть разными способами содержимое директорий и файлов
    * Операции с файлами и каталогами - удалять, создавать, редактировать, находить
    * Научиться работать с командами - создать свои, как работать с мануалами и утилитами командной строки.
    * Перенаправление - разобраться в >, >>, |, ||, && и прочие вариации
    * Особенности работы с клавиатурой - основные горячие клавиши, чтобы было проще работать в командной строке
    * Символические и жёсткие ссылки
    * Cron - что это, как настроить, основные параметры
3. Привилегии
    * Изучить основные понятия - sudo, пароли, группы и пользователи
    * Как настраивается sudo?
    * Какие есть права у файлов? Как устанавливаются права на файлы? 
4. Работа с текстом (можно на примере шаблонизации yaml и json сделать задачи)
    * Обработка текста - вырезать, выводить, сортировать, фильтровать строки в тексте
    * Форматирование вывода - как в линуксе можно красиво выводить логи
5. Устройства хранения
    * Какие есть основные файловые системы и их преимущества
    * Как размечается диск, монтируется директория
    * Что такое lvm, зачем он нужен, изучить основные сущности и поразбираться с основными настройками
    * Что такое RAID-массивы
    * Что такое swap
    * Что такое Inode
6. Управление пакетами
    * Какие бывают пакетные менеджеры
    * Как устанавливаются пакеты и репозитории?
    * Как вывести список пакетов?
    * Как удалить пакет?
    * Какие инструменты для обеспечения безопасности используются в репозиториях и как настраиваются?
7. Сети в линуксе
    * Какие есть инструменты работы с сетью в командной оболочке
    * Как работают эти инструменты, основные флаги и команды
8. Процессы
    * Что такое процессы, как их найти, как убить
    * Что такое сигналы и какой у процессов lifecycle
    * Что такое зомби процессы
    * Как запускать процессы фоном
9. systemd
    * Как создать юнит файл
    * Как управлять сервисами
    * Какие есть сущности в systemd
    * Как вывести список systemd сервисов
    * Как создать deb/rpm пакет
10. Bash
    * Циклы
    * Условные выражения
    * Чтение с клавиатуры
    * Аргументы
    * Массивы

### Сети
2. TCP и UDP
    * Что такое TCP и UDP? В чем их основное назначение? Почему TCP называют надежным протоколом, а UDP — ненадежным? Какие типы данных лучше передавать через TCP, а какие через UDP?
    * Чем отличается установка соединения в TCP (three-way handshake) от подхода UDP?
    * Как обеспечивается гарантия доставки данных в TCP?
    * Какие заголовки есть у TCP и UDP? Чем они отличаются по размеру и структуре?
    * Как обрабатываются потерянные пакеты в TCP и UDP?
    * Как работает контроль перегрузки (congestion control) в TCP?
    * Что такое скользящее окно (sliding window) в TCP?
    * Как работает механизм повторной передачи (retransmission) в TCP?
    * Что такое сегмент TCP и как он формируется?
3. ICMP
    * Зачем надо, какие есть заголовки, инструменты отладки
4. SSH
    * Что это, на каком порте работает
    * Как подключиться к хосту по ssh
    * Как настроить ssh-config, чтобы не вводить всё руками
    * Что такое jump host
    * Какие есть настройки безопасности для ssh сервера?
    * Как передавать файлы по ssh
4. DNS
      * Что такое DNS и какую проблему решает?
      * Какие основные компоненты dns?
      * Что такое FQDN?
      * Какие существуют уровни доменов?
      * Какие типы DNS серверов существуют?
      * Что такое DNS зона?
      * Как устроена иерархия DNS?
      * Что такое домены верхнего уровня?
      * Какие основные типы DNS записей бывают и какую функцию выполняют?
      * Какие этапы проходит запрос при разрешении домена?
      * Что такое DNS-резолвер и как он взаимодействует с серверами?
      * Какие инструменты используются для работы с dns?
      * По каким протоколам работает dns?
6. http
      * Что такое http и для чего используется?
      * Какие основные версии http существуют и чем они отличаются? Какие преимущества даёт http/2?
      * Что такое URL, URI, URN? Чем отличаются?
      * Какие основные методы http и их назначение?
      * Чем отличаются безопасные и идемпотентные методы?
      * Для чего используется метод OPTIONS?
      * В чем разница между PUT и PATCH?
      * Когда применяется метод HEAD?
      * Какие классы HTTP-статусов и их назначение?
      * Какие заголовки управляют кешированием? (Cache-Control, ETag, Expires)
      * Как работают заголовки аутентификации? (Authorization, WWW-Authenticate)
      * Для чего нужны заголовки Content-Type и  Content-Length?
      * Что такое CORS и какие заголовки с ним связаны?
      * Чем https отличается от http? Как работает в контексте http TLS/SSL?
      * Какие методы аутентификации поддерживает http?
      * Как работает кеширование на стороне клиента и сервера?
      * Что такое ETag и как он используется для валидации кеша?
      * Как работають куки? Какие параметры управляют их жизненным циклом? Чем отличаются сессионные куки и постоянные куки?
      * Что такое REST и какие у него принципы?
      * Какие основные инструменты и сервисы для работы с http трафиком используются?
7. tls
    * Что такое TLS и зачем нужен? Чем отличается tls от ssl?
    * Какие основные задачи решает tls?
    * Какие версии tls существуют?
    * Какие типы шифрования используются в TLS?
    * Что такое публичные и приватные ключи, как сгенерировать, какие есть алгоритмы шифрования
    * Полный процесс работы tls (Key exchange) в https и tls handshake
    * Как используется tls при генерации сертификатов для удостоверяющих центров
8. gRPC
      * Что такое gRPC и каковы его основные преимущества?
      * Какие проблемы решает gRPC в сравнении с REST/HTTP API?
      * Какую роль в gRPC играет Protocol Buffers (Protobuf)?
      * Какие типы RPC поддерживает gRPC (унарные, серверные стримы, клиентские стримы, двусторонние стримы)?
      * Как описать сервис и сообщения в файле .proto?
      * Чем отличается синтаксис proto2 от proto3?
      * Как сгенерировать код клиента и сервера из .proto-файла?
      * Какие типы данных доступны в Protobuf и как они мапятся на типы в языках программирования?
      * Как происходит сериализация и десериализация данных в gRPC?
      * Как работает двунаправленная потоковая передача данных (bidirectional streaming)?
      * Как обрабатываются ошибки в gRPC (статусы, кастомные ошибки)?
      * Как передавать метаданные между клиентом и сервером?
      * Как настроить аутентификацию в gRPC (SSL/TLS, токены, mTLS)?
      * Какие механизмы авторизации поддерживает gRPC?
      * Как использовать grpcurl для тестирования gRPC-сервисов?
      * Какие инструменты помогают в дебаггинге gRPC-запросов?
      * Как работать с контекстом (context) в gRPC?
9. NAT и iptables
    * Что такое Source NAT, Destination NAT, Static NAT, Dynamic NAT, PAT. Изучить и настроить
    * Что такое iptables
    * Какие есть цепочки, методы, таблицы в iptables
    * Какой приоритет правил в iptables
    * Через какой полный путь внутри межсетевого экрана проходит пакет в линукс?

### Контейнеризация
1. Основы контейнеризации и архитектура Docker
      * Что такое контейнеризация и чем она отличается от виртуализации?
      * Какие преимущества дают контейнеры по сравнению с виртуальными машинами?
      * Что такое Docker?
      * Из каких компонентов состоит Docker?
      * Что такое образ, контейнер и Dockerfile?
      * Из чего состоит образ? Как работает файловая система контейнера (Union File System)?
      * Как докер использует пространства имён (namespaces) и контрольные группы? (cgroups)?
      * Что такое DockerHub и для чего он нужен? Какие ещё container registry бывают?
2. Работа с Docker и оптимизация образов
      * Как создать Docker-образ с помощью Dockerfile? Какие инструкции используются в Dockerfile?
      * Разница между CMD и ENTRYPOINT
      * Зачем нужен EXPOSE?
      * Разница между ADD и COPY
      * Какие инструкции создают слои в контейнерах?
      * Как уменьшить размер Docker-образа? Какие есть способы оптимизации слоёв?
      * Как запустить контейнер из образа и управлять его жизенным циклом?
      * Как просматривать список контейнеров, образов, вольюмов и очищать их?
      * Как передавать переменные среды в контейнер?
      * Как пробросить порты из контейнера на хост машину?
      * Что такое Docker volume и как работать с постоянными данными? Куда можно монтировать volume?
      * Какие есть драйверы логов?
      * Как подключиться к оболочке контейнера?
      * Как просматривать логи контейнера в реальном времени?
      * Как реализовать healthchecks для контейнеров? Что это и зачем нужно?
      * Как мигрировать базу данных в Docker?
      * Как ограничить ресурсы контейнера (CPU и память)?
3. Сеть в Docker
      * Какие сетевые драйверы доступны в Docker?
      * Как связать несколько контейнеров в одну сеть для взаимодействия?
      * Что такое docker compose? Как его использовать? Какие есть команды? Пример docker-compose.yml файла?
      * Как работает dns между контейнерами?

### Мониторинг
1. Основы мониторинга Linux
      * Что такое мониторинг и зачем он нужен в системах?
      * Какие типы мониторинга существуют?
      * Чем отличается активный мониторинг от пассивного?
      * Какие основные метрики мониторинга CPU в линукс? Какие отладочные инструменты использовать для их проверки?
      * Какие основные метрики мониторинга RAM в линуксе? Какие отладочные инструменты использовать для их проверки?
      * Какие основные метрики мониторинга диска и дискового пространства в линуксе? Какие отладочные инструменты использовать для их проверки?
      * Что показывает команда dmesg? Как фильтровать её вывод?
      * Метрики сетевой активности и утилиты для отладки
2. Логирование
      * Где хранятся все логи в Linux?
      * Как работать с journalctl?
      * Как настроить ротацию логов?
      * Что такое централизованный сбор логов и какие инструменты используются?
3. Инструменты мониторинга
      * Как работает Prometheus и какие бывают основные экспортеры?
      * Что такое Grafana и как используется в связке с Prometheus?
      * Как настраивается алертинг в Prometheus? Как избежать перегрузки систем мониторинга?

### Git
1. Основы Git
      * Что такое Git и для чего нужен?
      * Какие три состояния файлов существуют в Git?
      * Как создать Git репозиторий?
      * Что такое .gitignore и для чего он нужен?
      * Что такое коммит?
      * Как посмотреть историю коммитов?
      * Как добавить файлы в коммит?
      * Как посмотреть изменения в файлах до коммита?
      * Что такое ветка и как создать новую ветку?
      * Как переключаться между ветками?
      * Чем отличается git merge и git rebase?
      * Что такое конфликт слияния и как его решить?
      * Что такое cherry-pick?
2. Работа с удалёнными репозиториями
      * Как удалить ветку локально и в удалённом репозитории?
      * Как добавить удалённый репозиторий?
      * Чем отличается git fetch от git pull?
      * Как отправить изменения в удалённый репозиторий?
      * Что такое origin и upstream в контексте Git?
      * Как клонировать репозиторий?
3. Отмена изменений
      * Как отменить изменения в рабочей директории?
      * Как отменить индексацию файла?
      * Чем отличаются git reset soft, mixed и hard?
      * Для чего используется git revert и чем он отличается от git reset?
4. Тэги
      * Чем тэги отличаются от веток?
      * Как создать тэг? Какие тэги бывают?
      * Как перейти к конкретному тэгу?
      * Как удалить тэг локально и на удалённом репозитории?
5. Работа в команде
      * Что такое PR/MR и как его создать?
      * Какие есть практики написания коммитов?
      * Что такое git flow и какие бывают подходы?
      * Что такое code review и как его проводить с помощью git?

### Сервисы и подходы к DevOps
1. Сервисы
      * Как развернуть nginx, postgres, minio, vault, gitlab, nexus
      * Уметь настраивать nginx как http-, tcp-прокси, балансировщик, tls, веб-сервер
      * Уметь работать с postgres - создание бд, пользователей, настройка прав, дамп, основные настройки в конфиге postgres.conf
      * 
1. nginx
      * Что это? Основные сценарии использования?
      * Уметь развернуть и настроить http-, tcp-прокси, балансировщик, tls, веб-сервер
2. postgres
      * Уметь развернуть и настроить создание бд, пользователей, прав, импорт/экспорт данных, дамп и рестор. основные настройки оптимизации
3. minio
4. vault
5. gitlab
6. nexus
