FROM ubuntu:20.04
RUN apt update && apt install -y iproute2 iptables

# Включаем маршрутизацию
RUN echo "net.ipv4.ip_forward=1" >> /etc/sysctl.conf

# Скрипт запуска
COPY entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh
CMD ["/entrypoint.sh"]