#!/bin/bash

nohup ./rogue_ai.sh > /dev/null 2>&1 &
echo $! > rogue.pid