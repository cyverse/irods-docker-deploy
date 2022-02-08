#! /bin/bash
#
# Usage:
#  entrypoint [SVC-SCRIPT]
#
# Parameter:
#  SVC-SCRIPT  the absolute path to a service control script
#
# This script is the entrypoint used by docker when starting an env container.
# If a service control script is provided, it will be executed with 'start' as
# an argument. After that, sshd is started. When TCP port 22 becomes responsive,
# any services started by the control script should be ready. When a SIGTERM
# signal is received, sshd is terminated. If a control script was provided, it
# is executed again but with 'stop' as its argument.
#
# A service control script should be able to start and stop any service other
# than sshd provided by the container. It should accept 'start' and 'stop' as
# its first command line argument. When executed with 'start', it should start
# any services of interest. It should not exit until these services are ready.
# When executed with 'stop', it should stop any services started by when the
# 'start' option is passed in. It should not exit until these services have
# stopped.
#


main()
{
  local svc="$1"

  start "$svc"
  trap "kill \${!}; stop '$svc'; exit \$?" SIGTERM

  while true
  do
    tail --follow /dev/null &
    wait ${!}
  done
}


start()
{
  local svc="$1"

  if [ -n "$svc" ]
  then
    eval "$svc" start
  fi

  /usr/sbin/sshd

  printf 'ready\n'
}


stop()
{
  local svc="$1"

  kill -SIGTERM $(cat /var/run/sshd.pid)

  if [ -n "$svc" ]
  then
    eval "$svc" stop
  fi
}


set -e

main "$*"
