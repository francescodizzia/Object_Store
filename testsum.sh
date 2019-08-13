#!/bin/bash

TEST_NUMBER="$(grep -c "Test" testout.log)"
SUCCESSFUL_NUMBER="$(grep -c "successo" testout.log)"
FAILED_NUMER="$(grep -c "fallito" testout.log)"

echo "Client lanciati: ${TEST_NUMBER}"
echo "Completati con successo: ${SUCCESSFUL_NUMBER}"
echo "Falliti: ${FAILED_NUMER}"

USERS_FAIL="$(grep -s "fallito" testout.log | cut -d "[" -f 3 | cut -d " " -f 2 | cut -d "]" -f 1)"

if [ "${FAILED_NUMER}" != "0" ]
then
printf "\n\nUtenti che hanno fallito (guardare il file di log per maggiori dettagli):\n${USERS_FAIL}\n"
fi

PID="$(ps aux | grep ./server | grep -v grep | awk '{print $2}')"

kill -SIGUSR1 ${PID}
