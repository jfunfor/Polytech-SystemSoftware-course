FROM python:3.11-slim

RUN apt-get update && apt-get install -y gcc libpq-dev curl unzip && rm -rf /var/lib/apt/lists/*

COPY terraform_bin /usr/local/bin/terraform
RUN chmod +x /usr/local/bin/terraform

WORKDIR /app

# Копируем файл зависимостей для установки через pip
COPY requirements.txt /app/

# Создаем виртуальное окружение и устанавливаем зависимости
RUN python3 -m venv /opt/venv \
    && . /opt/venv/bin/activate \
    && pip install --upgrade pip \
    && pip install -r requirements.txt

# Копируем весь код приложения
COPY . .

COPY .terraformrc /root/.terraformrc

RUN mkdir -p /app/terraform/plugins/registry.terraform.io/telmate/proxmox/3.0.1-rc2/linux_amd64/

RUN cp /app/terraform-provider-proxmox_v3.0.1-rc2 /app/terraform/plugins/registry.terraform.io/telmate/proxmox/3.0.1-rc2/linux_amd64/terraform-provider-proxmox_v3.0.1-rc2 \
    && chmod +x /app/terraform/plugins/registry.terraform.io/telmate/proxmox/3.0.1-rc2/linux_amd64/terraform-provider-proxmox_v3.0.1-rc2

ARG PROXMOX_API_TOKEN_SECRET_ARG
ARG DATABASE_URL_ARG
ARG SSH_PRIVATE_KEY_ARG
ARG REDIS_PASS_ARG
ARG KEYCLOAK_URL_ARG
ARG KEYCLOAK_VM_MANAGER_ID_ARG
ARG KEYCLOAK_VM_MANAGER_SECRET_ARG
ARG KEYCLOAK_REALM_ARG

ENV KEYCLOAK_URL=${KEYCLOAK_URL_ARG}
ENV KEYCLOAK_VM_MANAGER_ID=${KEYCLOAK_VM_MANAGER_ID_ARG}
ENV KEYCLOAK_VM_MANAGER_SECRET=${KEYCLOAK_VM_MANAGER_SECRET_ARG}
ENV KEYCLOAK_REALM=${KEYCLOAK_REALM_ARG}

ENV DATABASE_URL=${DATABASE_URL_ARG}
ENV VM_TTL_SECONDS="3600"
ENV DOCKER_ENV="true"
ENV RECIPE_REPO_URL="git@github.com:jfunfor/Polytech-SystemSoftware-course.git"
ENV SSH_PRIVATE_KEY=${SSH_PRIVATE_KEY_ARG}

ENV PROXMOX_API_TOKEN_SECRET=${PROXMOX_API_TOKEN_SECRET_ARG}
ENV PROXMOX_API_TOKEN_ID="root@pam!vm-manager"
ENV PROXMOX_API_URL="http://10.160.163.103:8006/api2/json"

ENV REDIS_HOST="10.160.160.202"
ENV REDIS_PORT="6379"
ENV REDIS_PASS=${REDIS_PASS_ARG}

EXPOSE 8000

CMD ["/bin/sh", "-c", ". /opt/venv/bin/activate && alembic upgrade head && uvicorn src.vm_manager.main:app --host 0.0.0.0 --port 8000"]
