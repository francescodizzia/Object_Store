#!/bin/bash
FILE_PATH="./testFiles"

./client "pippo" 4 "${FILE_PATH}/image.png" "unipi.png" &
./client "pippo" 4 "${FILE_PATH}/image2.png" "unipi_2.png" &
./client "pippo" 4 "${FILE_PATH}/goomba.gif" "goomba.gif" &
./client "pippo" 4 "${FILE_PATH}/testoprogetto2019.pdf" "consegna.pdf" &

wait

