#!/bin/bash


for i in {1..1}
do
#  ./client "user_${i}" "msg_${i}" "store" &
  ./client "FrancescoDizzia" "msg_i_reg" "register" &
    ./client "StefanoRusso" "msg_i_reg" "register" &
#      ./client "GiovanniAlbertoAmato" "msg_i_reg" "register" &
#        ./client "GianalbertoDiMartino" "msg_i_reg" "register" &
#                ./client "Abcd" "msg_i_reg" "register" &

done


wait
echo "Done"
