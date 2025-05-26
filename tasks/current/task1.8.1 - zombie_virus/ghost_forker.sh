#!/bin/bash

# Ghost process: forker that creates subprocesses that don't get reaped.
# Simulates zombie processes if parent is killed improperly.

trap "" SIGTERM  # Игнорировать SIGTERM

while true; do
    bash -c 'sleep 10' &  # Создаем фонового потомка, который может стать зомби
    sleep 2
done