#!/bin/bash

for i in {1..3}
do
  ./client "user_${i}" &
done




wait
echo "Done"
