#!/bin/bash          

if [ -z "$1" ] 
then
PID="$(ps aux | grep ./server | grep -v grep | awk '{print $2}')"
if [ -z "$PID" ] 
then
echo "Server non avviato!"
else
htop -p ${PID}
fi
else
PID="$(ps aux | grep ./client | grep -v grep | awk '{print $2}')"
echo "$PID"
fi

