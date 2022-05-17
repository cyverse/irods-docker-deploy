#!/bin/bash
#
# Assigns a UUID to a collection or data object.
#
# USAGE:
#  set-uuid type path uuid
#
# PARAMETERS:
#  type  The type of entity receiving the UUID: `-C` for collection or `-d` for
#        data object.
#  path  The absolute path to the entity receiving the UUID.
#  uuid  The UUID being assigned.

# XXX - This is a workaround for https://github.com/irods/irods/issues/3437. It is still present in
#       4.2.2.

set -e


main()
{
  if [ "$#" -lt 1 ]
  then
    printf 'The type of the entity being given a UUID needs to be provided\n' >&2
    return 1
  fi

  if [ "$#" -lt 2 ]
  then
    printf 'The absolute path to the entity needs to be provided\n' >&2
    return 1
  fi

  if [ "$#" -lt 3 ]
  then
    printf 'The UUID to assign needs to be provided\n' >&2
    return 1
  fi

  local type="${1^^}"
  local path="$2"
  local uuid="$3"

  if [ "$type" != '-C' -a "$type" != '-d' ]
  then
    printf 'The entity type must be "-C" for a collection or "-d" for a data object.\n' >&2
    return 1
  fi

  imeta set "$type" "$path" ipc_UUID "$uuid"
}


main "$@"
