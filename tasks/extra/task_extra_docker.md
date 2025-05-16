# Октопус

Начинающий фронтендер Октопус разочаровался в своей работе и решл попробовать что-то для настоящих мужчин! Как жаль, что товарищ посоветовал ему стать DevOps-специалистом, при этом вписав в резюме кучу ложных сведений. Теперь вам надо помочь октопусу создать отказоустойчивую микросервисную платформу, используя Docker, docker-compose, пользовательские сети, volumes, многоступенчатые образы и инструменты мониторинга. Обеспечить безопасность, производительность и изоляцию компонентов.

> Предполагется, что на это задание будет написан отчет, а само решение после нескольких автотестов проверено преподавателем

# Условие задачи

Требуемая архитектура состоит из следующих компонентов:
1. **Nginx Reverse Proxy** - маршрутизирует запросы к фронтенду и бэкенду, Использует HTTPS с самоподписанным сертификатом (сгенерируйте его при сборке), конфигурация Nginx должна динамически подключать upstream-адреса бэкенда через переменные окружения.

2. **Фронтенд** Статический веб-интерфейс, обслуживаемый через Nginx. Общается с бэкендом через REST API.

3. **Бэкенд (Python/Flask)** - Предоставляет API для фронтенда. Подключается к PostgreSQL (основная БД) и Redis (кэш). Зависит от переменных окружения для секретов (пароли БД, ключи API).

4. **Celery Worker + RabbitMQ** - Worker логгирует полученные сообщения. RabbitMQ выступает в роли брокера сообщений.

5. **Мониторинг** - Prometheus собирает метрики с бэкенда, Celery и PostgreSQL. Grafana визуализирует данные через готовый дашборд.

6. **PostgreSQL + Redis** - Данные должны сохраняться в volumes. PostgreSQL настроен с репликацией (1 master, 1 replica).

# Требования

- Использовать `.dockerignore`
- Запускать контейнеры от не-root пользователей
- Использовать пользовательские сети для фронтенда, бэкенда, Celery и RabbitMQ, мониторгинга
- Запретить прямой доступ к PostgreSQL и Redis из внешних сетей.
- Все секреты вынести в .env-файл и добавить его в gitignore.

# Примеры фронтенда/бэкенда

В качестве примера фронтенда приведем статическую HTML-страницу с JS, которая делает запросы к API:

```html
<!DOCTYPE html>
<html>
<body>
  <h1>Docker Demo App</h1>
  <button onclick="fetchData()">Get Data</button>
  <div id="result"></div>
  <script>
    async function fetchData() {
      const response = await fetch('http://backend/api/data');
      const data = await response.json();
      document.getElementById('result').textContent = data.message;
    }
  </script>
</body>
</html>
```

В качестве бекенда используем простое приложение на Python+Flask:

```python
from flask import Flask, jsonify
app = Flask(__name__)

@app.route('/api/data')
def get_data():
    return jsonify({"message": "Hello from Docker!"})

if __name__ == '__main__':
    app.run(host='0.0.0.0')
```

## Celery Worker

```py
from celery import Celery
app = Celery('tasks', broker='pyamqp://guest@rabbitmq//')

@app.task
def log_message(message):
    print(f"Received message: {message}")
```


