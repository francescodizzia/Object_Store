#!/bin/bash
FILE_PATH="./testFiles"

./client "pippo" 4 "${FILE_PATH}/image.png" "unipi.png" &
./client "luigi" 4 "${FILE_PATH}/image2.png" "unipi_2.png" &
./client "mario" 4 "${FILE_PATH}/goomba.gif" "goomba.gif" &
./client "pluto" 4 "${FILE_PATH}/testoprogetto2019.pdf" "consegna.pdf" &

wait

PID="$(ps aux | grep ./server | grep -v grep | awk '{print $2}')"

kill -SIGUSR1 ${PID}
