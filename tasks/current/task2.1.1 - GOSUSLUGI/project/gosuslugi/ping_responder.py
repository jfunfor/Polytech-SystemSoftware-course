from scapy.all import *
import os

def handle_icmp(pkt):
    if ICMP in pkt and pkt[ICMP].type == 8:
        if b"пж" in bytes(pkt[Raw].load):
            ip = pkt[IP].src
            print(f"[+] Получен вежливый пинг от {ip}")
            reply = IP(dst=ip)/ICMP(type=0)/b"GOSU-PASS-4421"
            send(reply, verbose=0)

sniff(filter="icmp", prn=handle_icmp)