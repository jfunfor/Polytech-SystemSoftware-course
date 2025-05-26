FROM ubuntu:20.04
RUN apt update && apt install -y iproute2 iputils-ping hping3 net-tools curl
WORKDIR /root
COPY ping_gosu.sh .
RUN chmod +x ping_gosu.sh
CMD ["bash"]