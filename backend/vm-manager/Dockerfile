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

EXPOSE 8000

CMD ["/bin/sh", "-c", ". /opt/venv/bin/activate && alembic upgrade head && uvicorn src.vm_manager.main:app --host 0.0.0.0 --port 8000"]
