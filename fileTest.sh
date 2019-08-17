#!/bin/bash
FILE_PATH="./testFiles"

./client "pippo" 4 "${FILE_PATH}/image.png" "unipi.png" &
./client "luigi" 4 "${FILE_PATH}/image2.png" "unipi_2.png" &
./client "mario" 4 "${FILE_PATH}/goomba.gif" "goomba.gif" &

wait
