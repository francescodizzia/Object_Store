#!/bin/bash
exec > testout.log

for i in {1..50}
do
  ./client "user_${i}" 1 &
done
wait

for i in {1..50}
do
  if [ "${i}" -le "30" ]
  then
    ./client "user_${i}" 2 &
  else
    ./client "user_${i}" 3 &
  fi
done

wait
