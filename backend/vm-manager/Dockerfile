FROM python:3.13-slim

RUN apt-get update && apt-get install -y gcc libpq-dev curl unzip && rm -rf /var/lib/apt/lists/*

COPY terraform_bin /usr/local/bin/terraform
RUN chmod +x /usr/local/bin/terraform

RUN curl -sSL https://install.python-poetry.org | python3 -  
ENV PATH="/root/.local/bin:$PATH"

WORKDIR /app

COPY pyproject.toml poetry.lock* /app/
RUN poetry config virtualenvs.create false \
    && poetry install --no-interaction --no-ansi --no-root

COPY . .

COPY .terraformrc /root/.terraformrc

RUN mkdir -p /app/terraform/plugins/registry.terraform.io/telmate/proxmox/3.0.1-rc2/linux_amd64/

RUN cp /app/terraform-provider-proxmox_v3.0.1-rc2 /app/terraform/plugins/registry.terraform.io/telmate/proxmox/3.0.1-rc2/linux_amd64/terraform-provider-proxmox_v3.0.1-rc2 \
    && chmod +x /app/terraform/plugins/registry.terraform.io/telmate/proxmox/3.0.1-rc2/linux_amd64/terraform-provider-proxmox_v3.0.1-rc2

ARG PROXMOX_API_TOKEN_SECRET_ARG
ENV PROXMOX_API_TOKEN_SECRET=${PROXMOX_API_TOKEN_SECRET_ARG}

ENV DATABASE_URL="postgresql://postgres:zZ00000@host.docker.internal:5432/vm_manager"
ENV VM_TTL_SECONDS="3600"
ENV DOCKER_ENV="true"
ENV PROXMOX_API_TOKEN_ID="root@pam!vm-manager"
ENV PROXMOX_API_URL="http://10.160.163.103:8006/api2/json"

ENV REDIS_HOST="localhost"
ENV REDIS_PORT="6379"
ENV REDIS_PASS="redispass"

EXPOSE 8000

CMD ["uvicorn", "src.vm_manager.main:app", "--host", "0.0.0.0", "--port", "8000"]
