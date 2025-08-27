FROM python:3.11-slim

# Устанавливаем зависимости
RUN apt-get update && apt-get install -y \
    git \
    curl \
    nodejs \
    npm \
    mariadb-server \
    redis-server \
    wkhtmltopdf \
    && rm -rf /var/lib/apt/lists/*

# Устанавливаем yarn
RUN npm install -g yarn

# Рабочая директория
WORKDIR /home/frappe/frappe-bench

# Копируем проект
COPY . .

# Python-зависимости
RUN pip install --no-cache-dir -r requirements.txt

# Собираем фронт (если есть)
RUN cd apps/lms/frontend && yarn install && yarn build || echo "no frontend"

# Открываем порт
EXPOSE 8000

# Скрипт запуска: сначала MariaDB и Redis, потом Frappe
CMD service mariadb start && \
    service redis-server start && \
    bench start
