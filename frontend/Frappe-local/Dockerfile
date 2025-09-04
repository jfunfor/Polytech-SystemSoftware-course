FROM python:3.11-slim

# Устанавливаем системные зависимости (dev-пакеты для сборки Python-библиотек)
RUN apt-get update && apt-get install -y \
    git \
    curl \
    nodejs \
    npm \
    build-essential \
    gcc \
    libffi-dev \
    libssl-dev \
    libjpeg-dev \
    zlib1g-dev \
    libpq-dev \
    python3-dev \
    && rm -rf /var/lib/apt/lists/*

# Устанавливаем yarn
RUN npm install -g yarn

# Устанавливаем frappe-bench (CLI)
RUN pip install --no-cache-dir --prefer-binary frappe-bench

# Рабочая директория
WORKDIR /home/frappe/frappe-bench

# Копируем проект
COPY . .

# Устанавливаем Python-зависимости (с приоритетом бинарных колёс)
RUN pip install --no-cache-dir --prefer-binary -r requirements.txt

RUN useradd -ms /bin/bash nastya

RUN mkdir -p /home/frappe/frappe-bench/logs && \
    chown -R nastya:nastya /home/frappe/frappe-bench

# Собираем фронтенд (если есть)
RUN cd apps/lms/frontend && yarn install && yarn build || echo "no frontend"

# Открываем порт
EXPOSE 8000

# Запускаем только frappe (без mariadb/redis)
CMD ["bench", "start"]
