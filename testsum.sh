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
ERRORS="$(grep -s "ERROR" testout.log | cut -d ":" -f 2 | cut -c 2-)"
printf "\n\nUtenti che hanno fallito:\n${USERS_FAIL}\n\nErrori riscontrati (vedere il file di log):\n${ERRORS}\n"
fi
