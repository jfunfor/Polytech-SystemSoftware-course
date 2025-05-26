FROM debian:bookworm

RUN apt-get update && \
    apt-get install -y iproute2 iptables iputils-ping net-tools curl openssh-server && \
    apt-get clean

RUN echo "root:root" | chpasswd
RUN mkdir /var/run/sshd

CMD ["/usr/sbin/sshd", "-D"]