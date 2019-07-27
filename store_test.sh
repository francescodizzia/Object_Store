#!/bin/bash


for i in {1..100}
do
  #./client "user_${i}" "msg_${i}" "store" &
  ./client "user_${i}_R" "msg_${i}_reg" "register" &
done

wait
echo "Done"
