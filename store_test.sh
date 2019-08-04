#!/bin/bash

for i in {1..100}
do
  ./client "user_${i}" &
done

wait

for k in {1..5}
do
 ./client "user_fail" &

done

wait
echo "Done"
