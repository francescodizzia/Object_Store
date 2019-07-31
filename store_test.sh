#!/bin/bash



for i in {1..1}
do
#  ./client "user_${i}" "msg_${i}" "register"
  ./client "user_${i}" "msg_${i}12345" "store"
#  ./client "user_${i}" "./art.gif" "store"
# ./client "./video.mp4" "./dream.mp4" "store" &

done
#  ./client "fdizzia" "msg#${j}#${i}${i}${i}${i}${i}${i}_end_" "store"

#    ./client "Stefano_Russo" "msg_i_reg" "register" &
  #   ./client "Giovanni_Alberto_Amato" "msg_i_reg" "register" &
  #     ./client "Gianni_Di_Martino" "msg_i_reg" "register" &
  #       ./client "noobmaster_69" ${verylongmsg} "store" &
  #        ./client "kakyoin" "xREROx_xREROx_xREROx" "store" &
  #         ./client "dio_brando" ${muda_str} "store" &




wait
echo "Done"
