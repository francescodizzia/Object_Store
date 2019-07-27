#!/bin/bash


for i in {1..1}
do
#  ./client "user_${i}" "msg_${i}" "store" &
  ./client "Francesco_D'Izzia" "msg_i_reg" "register" &
    ./client "Stefano_Russo" "msg_i_reg" "register" &
     ./client "Giovanni_Alberto_Amato" "msg_i_reg" "register" &
       ./client "Gianni_Di_Martino" "msg_i_reg" "register" &
         ./client "Abcd" "msg_i_reg" "register" &

done


wait
echo "Done"
