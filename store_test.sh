#!/bin/bash


for i in {1..100}
do
#  ./client "user_${i}" "msg_${i}" "store" &
  ./client "user_${i}_90123456789012345678901X" "msg_${i}_reg" "register" &
done

wait
echo "Done"
