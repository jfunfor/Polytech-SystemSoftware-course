FROM ubuntu:20.04
RUN apt update && apt install -y iproute2 iptables
RUN sysctl -w net.ipv4.ip_forward=1
CMD ["bash"]