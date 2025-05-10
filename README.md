**Сервисы** 
Современные приложения состоят из множества компонентов: базы данных, веб-интерфейса, хранилища данных, системы управления доступом и CI/CD. Эти компоненты представлены в виде сервисов — программных решений, предоставляющих определённую функциональность другим приложениям или пользователям.
*Примеры:*  
- nginx — веб-сервер, прокси, балансировщик
- postgres — реляционная СУБД
- minio — объектное хранилище, совместимое с Amazon S3
- vault — безопасное хранилище секретов (token, пароли, ключи)
- gitlab — система DevOps-практик, включая CI/CD
- nexus — хранилище артефактов (бинарников, docker-образов и т.д.)
**Что такое «деплой» и его стадии**
*Деплой (развёртывание)* — это процесс установки, настройки и запуска сервиса в конкретной среде (локальной, тестовой, продакшн).
*Типичный жизненный цикл деплоя:*  
- Подготовка окружения: ОС, права, пользователи, порты, каталоги
- Установка: бинарные пакеты, docker, сборка из исходников
- Настройка: конфигурационные файлы, переменные среды, параметры запуска
- Интеграция: взаимодействие с другими сервисами
- Мониторинг и поддержка: логи, aliveness-checks, резервные копии

*По типу данных сервисы классифицируются:*
Службы хранения: PostgreSQL, MinIO, Redis
Службы обработки: GitLab CI, Jenkins, cron-сервисы
Службы доступа: Nginx, Vault, LDAP
*По роли в инфраструктуре:*
Ядро (core): базы данных, балансировщики, секрет-хранилища
Инструментальные: CI/CD, репозитории, мониторинг

**NGINX** – высокопроизводительный веб-сервер, способный работать как обратный прокси, балансировщик нагрузки, TLS-терминатор.
Эффективен и использует событийную модель, поддерживает Lua, модули, а также работает с API, веб-приложениями, потоками данных.
*Установка и запуск:*
```
sudo apt update
sudo apt install nginx
sudo systemctl enable --now nginx
```
Конфигурация GINX:
Как веб-сервер:
```
server {
    listen 80;
    server_name example.com;
    root /var/www/html;
    index index.html;
}
```
HTTP-прокси:
```
server {
    listen 80;
    location / {
        proxy_pass http://localhost:8080;
    }
}
```
TCP-прокси:
```
stream {
    server {
        listen 5432;
        proxy_pass 127.0.0.1:5432;
    }
}
```
Блокировка нагрузки:
```
upstream backend {
    server backend1.example.com;
    server backend2.example.com;
}

server {
    listen 80;
    location / {
        proxy_pass http://backend;
    }
}
```
TLS:
```
server {
    listen 443 ssl;
    ssl_certificate /etc/nginx/ssl/cert.pem;
    ssl_certificate_key /etc/nginx/ssl/key.pem;

    location / {
        proxy_pass http://localhost:8080;
    }
}
```

**PostgreSQL** — объектно-реляционная СУБД с открытым исходным кодом. *Основные плюсы:*  
-	ACID-согласованность
-	Расширяемость (пользовательские типы, функции)
-	Поддержка сложных SQL-запросов и индексов
-	Расширения: PostGIS, TimescaleDB и др.
*Типичные роли:*  
- централизованное хранилище данных
- аналитическая БД
- Бэкенд для веб-приложений и API
*Принципы настройки PostgreSQL:*
- Разделение прав доступа через роли
- Настройка сети в pg_hba.conf и postgresql.conf
- Регулярное резервное копирование (pg_dump, pg_basebackup)
- Мониторинг (pg_stat*, логирование)

*Установка и запуск:*
```
sudo apt install postgresql postgresql-contrib
sudo systemctl enable --now postgresql
```
*Подключение под суперпользователем*
```
sudo -u postgres psql
```
*Создание БД и пользователя*
```
CREATE DATABASE mydb;
CREATE USER myuser WITH PASSWORD 'mypassword';
GRANT ALL PRIVILEGES ON DATABASE mydb TO myuser;
```
*Резервное копирование и восстановление*
```
pg_dump mydb > dump.sql
psql mydb < dump.sql
```
*Основные параметры в postgresql.conf*
```
listen_addresses = '*'
max_connections = 100
shared_buffers = 128MB
logging_collector = on
log_directory = 'log'
```
**MinIO**
*MinIO* — это высокопроизводительное объектное хранилище с поддержкой S3 API. Оно применяется для хранения неструктурированных данных: файлов, изображений, журналов, резервных копий и т.д. Отлично подходит для локальной инфраструктуры и edge-решений.
*Применение:*
- Хранилище логов, дампов, образов
- Бэкап-репозитории
- Статическое файловое хранилище

*Установка и запуск(одиночный режим):*
``` 
wget https://dl.min.io/server/minio/release/linux-amd64/minio  
chmod +x minio                                                  
sudo mv minio /usr/local/bin/                                   
MINIO_ROOT_USER=admin MINIO_ROOT_PASSWORD=secret \
  minio server /data --console-address ":9001"  
```
*Vault* — система управления секретами
HashiCorp Vault — это инструмент управления доступом к секретным данным. Он позволяет централизованно управлять паролями, ключами, токенами и предоставлять их приложениям по запросу.
*Применение:*
- Безопасное хранение конфигураций и секретов
- Выдача временных учетных данных
- Хранилище TLS-сертификатов
*Установка и запуск (dev-режим):*
sudo apt install vault            
vault server -dev                 
``
**GitLab** — DevOps-платформа
GitLab — это комплексная DevOps-платформа: от хостинга репозиториев до CI/CD, управления задачами и релизами. Она включает в себя собственную БД, Redis, NGINX и удобный веб-интерфейс.
*Применение:*
- Контроль версий и совместная разработка
- CI/CD пайплайны
- Управление задачами и релизами
*Установка (Omnibus-версия):*
```
curl https://packages.gitlab.com/install/repositories/gitlab/gitlab-ee/script.deb.sh | sudo bash
sudo EXTERNAL_URL="http://gitlab.example.com" apt install gitlab-ee      
```
**Nexus** — репозиторий артефактов
Sonatype Nexus Repository Manager — это система хранения бинарных артефактов (jar, npm, docker, helm и др.). Используется для локального кэширования внешних репозиториев и хранения внутренних пакетов.
*Применение:*
- Артефактный менеджер для CI/CD пайплайнов
- Приватные docker- и helm-репозитории
- Кэширование внешних репозиториев (Maven, PyPi, npm)
*Установка и запуск:*
```
wget https://download.sonatype.com/nexus/3/latest-unix.tar.gz  
tar -xvzf latest-unix.tar.gz                                   
cd nexus-*                                                     
./bin/nexus start
```

