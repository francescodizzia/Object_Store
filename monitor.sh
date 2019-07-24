#!/bin/bash          

PID="$(ps aux | grep ./server | grep -v grep | awk '{print $2}')"

if [ -z "$PID" ] 
then
echo "Server non avviato!\n"
else
htop -p ${PID}
fi
