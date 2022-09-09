#!/bin/bash
#
# This script creates the *.re rules for an catalog service provider.
#
# It requires the following environment variables to be defined
#
# BISQUE_URL                The BisQue Service URL, https://bisque.ucsb.edu
# BISQUE_ADMIN_USER         The admin username
# BISQUE_ADMIN_PASSWORD     the admin user password


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
s/\$BISQUE_URL/$(escape $BISQUE_URL)/g
s/\$BISQUE_ADMIN_USER/$(escape $BISQUE_ADMIN_USER)/g
s/\$BISQUE_ADMIN_PASSWORD/$(escape $BISQUE_ADMIN_PASSWORD)/g
EOF
}


set -e
main
