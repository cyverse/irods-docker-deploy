#!/bin/bash
#
# This script creates the *.re rules for an catalog service provider.
#
# It requires the following environment variables to be defined
#
# IRODS_MAX_NUM_RE_PROCS  The max number of rule engine processes


main()
{
  mkdir -p /etc/bisque
  expand_tmpl > /etc/bisque/bisque_config
}


# escapes / and \ for sed script
escape()
{
  local var="$*"

  # Escape \ first to avoid escaping the escape character, i.e. avoid / -> \/ -> \\/
  var="${var//\\/\\\\}"

  printf '%s' "${var//\//\\/}"
}


expand_tmpl()
{
  cat <<EOF | sed --file - /tmp/bisque_config.template
s/\$BISQUE_HOST/$(escape $BISQUE_HOST)/g
s/\$BISQUE_ADMIN_USER/$(escape $BISQUE_ADMIN_USER)/g
s/\$BISQUE_ADMIN_PASSWORD/$(escape $BISQUE_ADMIN_PASSWORD)/g
EOF
}


set -e
main
