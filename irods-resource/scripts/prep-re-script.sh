#!/bin/bash
#
# This script creates the *.re rules for an catalog service provider.
#
# It requires the following environment variables to be defined
#
# IRODS_MAX_NUM_RE_PROCS  The max number of rule engine processes
# IRODS_ZONE_NAME         The zone name of iRODS service
# IRODS_BISQUE_ADMIN_USER
# BISQUE_IRODS_BASE_URL


main()
{
  expand_ipc_tmpl > /etc/irods/ipc-env.re
  expand_bisque_tmpl > /etc/irods/bisque-env.re
}


# escapes / and \ for sed script
escape()
{
  local var="$*"

  # Escape \ first to avoid escaping the escape character, i.e. avoid / -> \/ -> \\/
  var="${var//\\/\\\\}"

  printf '%s' "${var//\//\\/}"
}


expand_ipc_tmpl()
{
  cat <<EOF | sed --file - /tmp/ipc-env.re.template
s/\$IRODS_MAX_NUM_RE_PROCS/$(escape $IRODS_MAX_NUM_RE_PROCS)/g
s/\$IRODS_ZONE_NAME/$(escape $IRODS_ZONE_NAME)/g
EOF
}

expand_bisque_tmpl()
{
  cat <<EOF | sed --file - /tmp/bisque-env.re.template
s/\$BISQUE_IRODS_BASE_URL/$(escape $BISQUE_IRODS_BASE_URL)/g
s/\$IRODS_BISQUE_ADMIN_USER/$(escape $IRODS_BISQUE_ADMIN_USER)/g
EOF
}

set -e
main
