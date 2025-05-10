
##  Nginx и PostgreSQL

### Часть 1: Nginx

#### Что такое Nginx?

Nginx (произносится как "энджин-экс") — это высокопроизводительный веб-сервер, обратный прокси-сервер, а также почтовый прокси-сервер и универсальный TCP/UDP прокси-сервер. Он был создан Игорем Сысоевым в 2004 году. Nginx известен своей стабильностью, богатым набором функций, простой конфигурацией и низким потреблением ресурсов. Благодаря своей архитектуре, основанной на событиях (event-driven), Nginx может обрабатывать тысячи одновременных соединений с минимальными затратами памяти.

#### Основные сценарии использования Nginx:

1.  **Веб-сервер:** Обслуживание статического контента (HTML, CSS, JavaScript, изображения) и динамического контента через FastCGI, uWSGI, SCGI и другие протоколы для взаимодействия с серверными приложениями (например, написанными на PHP, Python, Ruby).
2.  **Обратный прокси-сервер (Reverse Proxy):** Принимает запросы от клиентов и перенаправляет их на один или несколько внутренних серверов. Это помогает скрыть структуру внутренней сети, распределить нагрузку, кэшировать контент и обеспечить SSL/TLS шифрование.
3.  **Балансировщик нагрузки (Load Balancer):** Распределяет входящий трафик между несколькими серверами приложений для повышения производительности, отказоустойчивости и масштабируемости.
4.  **HTTP-кэширование:** Хранение часто запрашиваемых данных для уменьшения нагрузки на серверы приложений и ускорения ответа клиентам.
5.  **SSL/TLS-терминация:** Обработка SSL/TLS соединений, снимая эту нагрузку с внутренних серверов приложений.
6.  **TCP/UDP прокси-сервер:** Проксирование трафика для различных сетевых протоколов, а не только HTTP.
7.  **API-шлюз (API Gateway):** Как точка входа для API, управляя маршрутизацией, аутентификацией, ограничением скорости и другими аспектами.

#### Развертывание и настройка Nginx

**1. Установка Nginx (пример для Debian/Ubuntu):**

```bash
sudo apt update
sudo apt install nginx
```

После установки Nginx обычно запускается автоматически. Вы можете проверить его статус:

```bash
sudo systemctl status nginx
```

**2. Основные файлы и директории конфигурации:**

*   `/etc/nginx/nginx.conf`: Основной конфигурационный файл.
*   `/etc/nginx/sites-available/`: Директория для хранения конфигурационных файлов виртуальных хостов (серверных блоков).
*   `/etc/nginx/sites-enabled/`: Директория, содержащая символические ссылки на конфигурационные файлы из `sites-available/`, которые Nginx будет использовать.
*   `/var/log/nginx/`: Директория с лог-файлами Nginx (access.log, error.log).
*   `/var/www/html/`: Директория по умолчанию для статического контента.

**3. Настройка как веб-сервера (обслуживание статического контента):**

По умолчанию Nginx уже настроен для обслуживания файлов из `/var/www/html/`. Вы можете создать простой HTML-файл:

```bash
sudo nano /var/www/html/index.html
```

Добавьте в него:

```html
<!DOCTYPE html>
<html>
<head>
    <title>Welcome to Nginx!</title>
</head>
<body>
    <h1>Success! The Nginx server is working!</h1>
</body>
</html>
```

Сохраните файл. Теперь, если вы перейдете в браузере по IP-адресу вашего сервера, вы должны увидеть эту страницу.

Пример конфигурационного блока сервера (`server block`) для простого веб-сайта в файле `/etc/nginx/sites-available/example.com`:

```nginx
server {
    listen 80;
    listen [::]:80;

    server_name example.com www.example.com; # Замените на ваше доменное имя

    root /var/www/example.com/html; # Путь к файлам вашего сайта
    index index.html index.htm;

    location / {
        try_files $uri $uri/ =404;
    }

    access_log /var/log/nginx/example.com.access.log;
    error_log /var/log/nginx/example.com.error.log;
}
```

После создания файла создайте символическую ссылку и проверьте конфигурацию:

```bash
sudo ln -s /etc/nginx/sites-available/example.com /etc/nginx/sites-enabled/
sudo nginx -t
sudo systemctl reload nginx
```

**4. Настройка как HTTP-прокси (Reverse Proxy):**

Предположим, у вас есть приложение, работающее на `localhost:3000`. Nginx может принимать внешние запросы и перенаправлять их на это приложение.

Пример конфигурации в блоке `server`:

```nginx
server {
    listen 80;
    server_name your_domain.com; # Ваше доменное имя

    location / {
        proxy_pass http://localhost:3000; # Адрес вашего внутреннего приложения
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;
    }
}
```

*   `proxy_pass`: Указывает адрес, на который будут перенаправляться запросы.
*   `proxy_set_header`: Позволяет передавать оригинальные заголовки запроса внутреннему приложению.

**5. Настройка как TCP-прокси:**

Для проксирования TCP-трафика (например, для баз данных или других служб) Nginx должен быть скомпилирован с модулем `ngx_stream_core_module` (обычно включен в стандартные сборки).

Конфигурация для TCP-проксирования добавляется вне блока `http`, обычно в основном файле `nginx.conf` или в отдельном файле, подключаемом через `include`.

```nginx
stream {
    upstream tcp_backend {
        server backend1.example.com:1234; # Адрес и порт вашего TCP-сервера
        # server backend2.example.com:1234; # Можно добавить несколько для балансировки
    }

    server {
        listen 5000; # Порт, на котором Nginx будет слушать TCP-соединения
        proxy_pass tcp_backend;
        proxy_timeout 300s;
        proxy_connect_timeout 5s;
    }
}
```

Здесь Nginx будет слушать TCP-соединения на порту 5000 и перенаправлять их на `backend1.example.com:1234`.

**6. Настройка как балансировщика нагрузки:**

Nginx может распределять HTTP или TCP/UDP трафик между несколькими серверами.

**HTTP балансировка:**

```nginx
http {
    upstream my_app_backend {
        # Алгоритмы балансировки: round-robin (по умолчанию), least_conn, ip_hash
        # server app1.example.com weight=3; # weight - вес сервера
        # server app2.example.com;
        # server app3.example.com;

        # Пример с IP Hash для "липких" сессий
        # ip_hash;
        server app1.example.com:8080;
        server app2.example.com:8080;
        server app3.example.com:8080;
    }

    server {
        listen 80;
        server_name your_domain.com;

        location / {
            proxy_pass http://my_app_backend;
            proxy_set_header Host $host;
            proxy_set_header X-Real-IP $remote_addr;
            proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
            proxy_set_header X-Forwarded-Proto $scheme;
        }
    }
}
```

*   Блок `upstream` определяет группу серверов.
*   `proxy_pass` указывает на эту группу.
*   Nginx поддерживает различные методы балансировки:
    *   `round-robin`: Запросы распределяются по серверам поочередно (по умолчанию).
    *   `least_conn`: Новый запрос отправляется на сервер с наименьшим количеством активных соединений.
    *   `ip_hash`: Запросы с одного IP-адреса всегда направляются на один и тот же сервер (полезно для сессий).
    *   `hash`: Балансировка на основе указанного ключа (например, URI запроса).

**TCP балансировка (аналогично TCP-прокси, но с несколькими серверами в `upstream`):**

```nginx
stream {
    upstream my_tcp_service {
        # least_conn; # Можно указать метод балансировки
        server service1.example.com:5432;
        server service2.example.com:5432;
    }

    server {
        listen 12345; # Внешний порт
        proxy_pass my_tcp_service;
    }
}
```

**7. Настройка TLS (HTTPS):**

Для настройки HTTPS вам понадобится SSL/TLS сертификат. Вы можете получить его от центра сертификации (CA) или использовать бесплатные сертификаты от Let's Encrypt.

Предположим, у вас есть файлы сертификата (`your_domain.crt`) и приватного ключа (`your_domain.key`).

Пример конфигурации сервера для HTTPS:

```nginx
server {
    listen 443 ssl http2; # Слушаем порт 443 для SSL, http2 для производительности
    listen [::]:443 ssl http2;
    server_name your_domain.com www.your_domain.com;

    root /var/www/your_domain.com/html;
    index index.html index.htm;

    ssl_certificate /etc/nginx/ssl/your_domain.crt; # Путь к вашему сертификату
    ssl_certificate_key /etc/nginx/ssl/your_domain.key; # Путь к вашему приватному ключу

    # Рекомендуемые настройки SSL
    ssl_protocols TLSv1.2 TLSv1.3;
    ssl_prefer_server_ciphers on;
    ssl_ciphers 'ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-CHACHA20-POLY1305:ECDHE-RSA-CHACHA20-POLY1305:DHE-RSA-AES128-GCM-SHA256:DHE-RSA-AES256-GCM-SHA384';
    ssl_session_timeout 1d;
    ssl_session_cache shared:SSL:10m; # Примерно 40000 сессий
    ssl_session_tickets off;
    # ssl_dhparam /etc/nginx/ssl/dhparam.pem; # Если используете DHE шифры

    # HSTS (HTTP Strict Transport Security) - необязательно, но рекомендуется
    # add_header Strict-Transport-Security "max-age=63072000; includeSubDomains; preload";

    location / {
        try_files $uri $uri/ =404;
    }

    # ... другие настройки ...
}

# Редирект с HTTP на HTTPS (необязательно, но рекомендуется)
server {
    listen 80;
    listen [::]:80;
    server_name your_domain.com www.your_domain.com;
    return 301 https://$server_name$request_uri;
}
```

**Важные команды Nginx:**

*   `sudo nginx -t`: Проверка синтаксиса конфигурационных файлов.
*   `sudo systemctl reload nginx`: Перезагрузка конфигурации без остановки сервера.
*   `sudo systemctl restart nginx`: Полный перезапуск сервера.
*   `sudo systemctl stop nginx`: Остановка сервера.
*   `sudo systemctl start nginx`: Запуск сервера.

### Часть 2: PostgreSQL

PostgreSQL — это мощная, объектно-реляционная система управления базами данных (ОРСУБД) с открытым исходным кодом, известная своей надежностью, гибкостью и соответствием стандартам SQL.

#### Развертывание и настройка PostgreSQL

**1. Установка PostgreSQL (пример для Debian/Ubuntu):**

```bash
sudo apt update
sudo apt install postgresql postgresql-contrib
```

После установки PostgreSQL сервер обычно запускается автоматически.

**2. Подключение к PostgreSQL:**

По умолчанию создается пользователь `postgres` с правами суперпользователя. Подключиться к командной строке PostgreSQL (`psql`) можно так:

```bash
sudo -u postgres psql
```

Вы попадете в интерактивную оболочку `psql`. Для выхода введите `\q`.

**3. Создание базы данных:**

Внутри `psql` (под пользователем `postgres` или другим пользователем с соответствующими правами):

```sql
CREATE DATABASE mydatabase;
```

Или из командной строки Linux:

```bash
sudo -u postgres createdb mydatabase
```

**4. Создание пользователей и прав:**

**Создание пользователя (роли):**

Внутри `psql`:

```sql
CREATE USER myuser WITH PASSWORD 'mypassword';
-- Или, для создания пользователя, который может входить в систему:
-- CREATE ROLE myuser LOGIN PASSWORD 'mypassword';
```

**Предоставление прав пользователю на базу данных:**

```sql
-- Предоставить все права на базу данных 'mydatabase' пользователю 'myuser'
GRANT ALL PRIVILEGES ON DATABASE mydatabase TO myuser;
```

**Предоставление прав на конкретные таблицы (если они уже существуют):**

```sql
-- Подключитесь к нужной базе данных: \c mydatabase
-- Предоставить права на выборку, вставку, обновление, удаление для таблицы 'mytable'
GRANT SELECT, INSERT, UPDATE, DELETE ON TABLE mytable TO myuser;
-- Предоставить права на все таблицы в схеме 'public' (часто используется по умолчанию)
GRANT ALL PRIVILEGES ON ALL TABLES IN SCHEMA public TO myuser;
GRANT ALL PRIVILEGES ON ALL SEQUENCES IN SCHEMA public TO myuser; -- Для автоинкрементных полей
GRANT ALL PRIVILEGES ON ALL FUNCTIONS IN SCHEMA public TO myuser;
```

**Изменение владельца базы данных:**

```sql
ALTER DATABASE mydatabase OWNER TO myuser;
```

**5. Импорт/Экспорт данных:**

**Экспорт (Dump):**

*   **`pg_dump`** - утилита для создания резервной копии (дампа) базы данных.

    *   **Экспорт всей базы данных в текстовом формате (SQL-команды):**
        ```bash
        pg_dump -U username -h hostname -p portnumber databasename > backup.sql
        # Пример для локального пользователя postgres:
        sudo -u postgres pg_dump mydatabase > mydatabase_backup.sql
        ```

    *   **Экспорт в кастомном формате (рекомендуется для больших баз, позволяет выборочное восстановление):**
        ```bash
        sudo -u postgres pg_dump -Fc mydatabase > mydatabase_backup.dump
        # -Fc означает "custom format"
        ```

    *   **Экспорт одной таблицы:**
        ```bash
        sudo -u postgres pg_dump -t mytable mydatabase > mytable_backup.sql
        ```

    *   **Экспорт только схемы (без данных):**
        ```bash
        sudo -u postgres pg_dump -s mydatabase > mydatabase_schema.sql
        ```

    *   **Экспорт только данных (без схемы):**
        ```bash
        sudo -u postgres pg_dump -a mydatabase > mydatabase_data.sql
        ```

**Импорт (Restore):**

*   **`psql`** - для восстановления из текстовых SQL-дампов.
*   **`pg_restore`** - для восстановления из кастомных, tar, или directory форматов, созданных `pg_dump`.

    *   **Импорт из текстового SQL-файла:**
        Сначала создайте пустую базу данных (если ее нет):
        ```bash
        sudo -u postgres createdb newdatabase
        ```
        Затем импортируйте:
        ```bash
        sudo -u postgres psql newdatabase < mydatabase_backup.sql
        ```
        Или внутри `psql`:
        ```sql
        \c newdatabase
        \i /path/to/mydatabase_backup.sql
        ```

    *   **Импорт из кастомного формата (`.dump`):**
        Сначала создайте пустую базу данных (если ее нет).
        ```bash
        sudo -u postgres createdb newdatabase
        ```
        Затем используйте `pg_restore`:
        ```bash
        sudo -u postgres pg_restore -d newdatabase mydatabase_backup.dump
        # -d указывает целевую базу данных
        ```
        Если дамп содержит команды создания базы данных или вы хотите восстановить с определенными опциями, `pg_restore` очень гибок.

**6. Дамп и Рестор (Резервное копирование и восстановление):**

Это по сути те же команды `pg_dump` и `pg_restore`/`psql`, которые описаны выше.

**Ключевые моменты для резервного копирования:**

*   **Регулярность:** Настройте автоматическое резервное копирование (например, через cron).
*   **Хранение:** Храните резервные копии в безопасном месте, желательно на другом физическом носителе или в облаке.
*   **Тестирование восстановления:** Периодически проверяйте, что вы можете успешно восстановить данные из резервных копий.
*   **Типы дампов:**
    *   **Логический дамп (`pg_dump`):** Создает SQL-скрипты или архивные файлы. Гибкий, позволяет восстанавливать на разных версиях PostgreSQL и архитектурах.
    *   **Физическое резервное копирование (File system level backup):** Копирование файлов данных PostgreSQL. Быстрее для очень больших баз, но менее гибкое (обычно требует той же версии и архитектуры).
    *   **Непрерывное архивирование и восстановление на определенный момент времени (Point-in-Time Recovery - PITR):** Использует базовую резервную копию и архивированные WAL (Write-Ahead Logging) файлы для восстановления состояния базы данных на любой момент времени. Это более сложная, но очень мощная стратегия.

**Пример простого скрипта для ежедневного бэкапа:**

```bash
#!/bin/bash
DB_NAME="mydatabase"
BACKUP_DIR="/var/backups/postgresql"
DATE=$(date +%Y-%m-%d_%H-%M-%S)

# Создаем директорию, если не существует
mkdir -p $BACKUP_DIR

# Делаем дамп
sudo -u postgres pg_dump -Fc $DB_NAME > "$BACKUP_DIR/${DB_NAME}_${DATE}.dump"

# (Опционально) Удаляем старые бэкапы, например, старше 7 дней
find $BACKUP_DIR -type f -name "*.dump" -mtime +7 -delete
```
Не забудьте сделать скрипт исполняемым (`chmod +x script.sh`) и добавить его в cron.

**7. Основные настройки оптимизации PostgreSQL:**

Файл конфигурации PostgreSQL обычно находится в `/etc/postgresql/<version>/main/postgresql.conf`. После изменений в этом файле требуется перезагрузка сервера PostgreSQL (`sudo systemctl restart postgresql`).

**Важно:** Прежде чем изменять эти параметры, тщательно изучите их влияние. Неправильные настройки могут ухудшить производительность или привести к нестабильной работе. Начинайте с небольших изменений и тестируйте.

*   **`shared_buffers`**:
    *   Определяет объем памяти, который PostgreSQL использует для кэширования данных.
    *   Рекомендация: обычно 25% от общего объема оперативной памяти системы. Для выделенных серверов баз данных может быть больше (до 40%).
    *   Пример: `shared_buffers = 2GB` (если у вас 8GB RAM).

*   **`work_mem`**:
    *   Объем памяти, используемый для внутренних операций сортировки, хэширования и других операций перед записью на диск.
    *   Устанавливается для каждого соединения/операции. Слишком высокое значение может привести к нехватке памяти, если много одновременных сложных запросов.
    *   Начинайте с небольших значений (например, 4MB-16MB) и увеличивайте, если видите, что сложные запросы часто пишут временные файлы на диск (можно отслеживать через `EXPLAIN ANALYZE`).
    *   Пример: `work_mem = 16MB`

*   **`maintenance_work_mem`**:
    *   Объем памяти, используемый для операций обслуживания, таких как `VACUUM`, `CREATE INDEX`, `ALTER TABLE ADD FOREIGN KEY`.
    *   Может быть установлен выше, чем `work_mem`, так как эти операции обычно не выполняются параллельно в большом количестве.
    *   Пример: `maintenance_work_mem = 256MB`

*   **`effective_cache_size`**:
    *   Оценка общего объема памяти, доступного для кэширования данных (включая `shared_buffers` и кэш операционной системы).
    *   Помогает планировщику запросов оценить, будут ли данные в кэше.
    *   Рекомендация: 50-75% от общего объема RAM.
    *   Пример: `effective_cache_size = 6GB` (если у вас 8GB RAM).

*   **`wal_buffers`**:
    *   Размер буфера для WAL (Write-Ahead Log) записей перед их записью на диск.
    *   Значение по умолчанию (`-1`) обычно устанавливает его в `1/32` от `shared_buffers` (но не менее `64kB` и не более `16MB`). Обычно значение по умолчанию подходит. Увеличение может помочь при высокой нагрузке на запись.
    *   Пример: `wal_buffers = 16MB` (если `shared_buffers` достаточно большой).

*   **`checkpoint_completion_target`**:
    *   Определяет, как быстро должен быть завершен процесс контрольной точки (checkpoint) в процентах от времени между контрольными точками.
    *   Значение `0.9` (по умолчанию `0.5` до PostgreSQL 9.5, `0.9` с 9.6+) распределяет нагрузку ввода-вывода от checkpoint на более длительный период, сглаживая пики.
    *   Пример: `checkpoint_completion_target = 0.9`

*   **`random_page_cost`**:
    *   Стоимость чтения случайной страницы с диска для планировщика запросов.
    *   Если у вас быстрые диски (SSD), вы можете уменьшить это значение (по умолчанию `4.0`) до `1.1` - `2.0`, чтобы планировщик чаще выбирал индексное сканирование.
    *   Пример: `random_page_cost = 1.1` (для SSD)

*   **`seq_page_cost`**:
    *   Стоимость чтения последовательной страницы с диска (по умолчанию `1.0`). Обычно не требует изменения.

*   **`max_connections`**:
    *   Максимальное количество одновременных подключений к серверу.
    *   Каждое соединение потребляет память. Увеличивайте с осторожностью. Часто лучше использовать пул соединений (connection pooler) вроде PgBouncer или Pgpool-II, чем устанавливать очень большое значение `max_connections`.
    *   Пример: `max_connections = 200`

*   **`autovacuum`**:
    *   Автоматический процесс очистки "мертвых" кортежей и анализа таблиц. Очень важно, чтобы он был включен (`autovacuum = on` по умолчанию). Настройки `autovacuum_max_workers`, `autovacuum_naptime` и пороговые значения для таблиц могут потребовать тюнинга для очень активных баз данных.

**Инструменты для анализа производительности:**

*   `EXPLAIN ANALYZE query;`: Показывает план выполнения запроса и реальное время выполнения. Используйте для выявления "узких мест" в запросах.
*   `pg_stat_statements`: Модуль, который отслеживает статистику выполнения всех SQL-запросов. Требует добавления в `shared_preload_libraries` в `postgresql.conf` и создания расширения (`CREATE EXTENSION pg_stat_statements;`).
*   `pg_buffercache`: Модуль для просмотра содержимого `shared_buffers`.
*   Логи PostgreSQL: Содержат информацию об ошибках, медленных запросах (`log_min_duration_statement`), блокировках и т.д.

