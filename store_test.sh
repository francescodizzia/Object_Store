#!/bin/bash

verylongmsg="AAABBBCCCDDDEEEEFFFFGGGGHHHHHIIIILLLLLMMMMMNNNNNOOOOOOPPPPPQQQQQQQRRRRSSSSSSSSSSSSTTTTTTTTUUUUUUUUXXXXXXXXYYYYYYYYYYYZZZZZ"
muda_str=""

for j in {1..200}
do
muda_str="${muda_str}_MUDA"
done

for j in {0..9}
do
#  ./client "user_${i}" "msg_${i}" "store" &
for i in {0..9}
do
  ./client "fdizzia" "msg#${j}#${i}${i}${i}${i}${i}${i}_end_" "store" &
done
#    ./client "Stefano_Russo" "msg_i_reg" "register" &
  #   ./client "Giovanni_Alberto_Amato" "msg_i_reg" "register" &
  #     ./client "Gianni_Di_Martino" "msg_i_reg" "register" &
  #       ./client "noobmaster_69" ${verylongmsg} "store" &
  #        ./client "kakyoin" "xREROx_xREROx_xREROx" "store" &
  #         ./client "dio_brando" ${muda_str} "store" &


done


wait
echo "Done"
