#! /bin/bash
#
# Usage:
#  service (start|stop)
#
# This script starts or stops the postgresSQL server inside the container.

set -o errexit


main()
{
  if [ "$#" -lt 1 ]
  then
    printf 'Requires either "start" or "stop" as its first parameter\n' >&2
    return 1
  fi

  local action="$1"

  if [ "$action" != start -a "$action" != stop ]
  then
    printf 'Unknown command "%s"\n' "$action" >&2
    return 1
  fi

  if id --user postgres &> /dev/null
  then
    sudo -i -u postgres pg_ctlcluster 12 main "$action"
  fi
}


main "$@"
