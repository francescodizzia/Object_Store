PID="$(ps aux | grep ./server | grep -v grep | awk '{print $2}')"
kill -SIGUSR1 ${PID}
