#!/bin/bash

for i in {1..100}
do
  ./client "user_${i}" &
done




wait
echo "Done"
