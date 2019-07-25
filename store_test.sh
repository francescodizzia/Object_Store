#!/bin/bash


for i in {1..1000}
do
  ./client "user_${i}" "msg_${i}" &
  #sleep 1
done

wait
echo "Done"
