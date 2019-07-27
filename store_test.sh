#!/bin/bash


for i in {1..5}
do
#  ./client "user_${i}" "msg_${i}" "store" &
  ./client "${i}rancescoDizzia" "msg_${i}_reg" "register" &
done

wait
echo "Done"
