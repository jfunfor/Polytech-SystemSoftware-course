
##  Nginx 


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

