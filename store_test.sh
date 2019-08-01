#!/bin/bash

for i in {1..1}
do
  ./client "user_${i}" &
done




wait
echo "Done"
