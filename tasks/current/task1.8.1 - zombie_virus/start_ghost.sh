#!/bin/bash

chmod +x ghost_forker.sh
nohup ./ghost_forker.sh > /dev/null 2>&1 &
echo $! > ghost.pid  # Сохраняем PID для автопроверки