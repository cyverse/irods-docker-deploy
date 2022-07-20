#!/bin/bash
#
# This script creates the sftpgo.json configuration file for SFTPGo service.
#
# It requires the following environment variables to be defined
#
# SFTP_PORT                       The TCP port for the sftp service
# SFTPGO_ADMIN_UI_PORT            The TCP port for web admin service
# SFTPGO_VAULT                    The local path for storing user data and log


main()
{
  expand_tmpl > /etc/sftpgo/sftpgo.json
  chmod a+r /etc/sftpgo/sftpgo.json
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
  cat <<EOF | sed --file - /tmp/sftpgo.json.template
s/\$SFTP_PORT/$(escape $SFTP_PORT)/g
s/\$SFTPGO_ADMIN_UI_PORT/$(escape $SFTPGO_ADMIN_UI_PORT)/g
s/\$SFTPGO_VAULT/$(escape $SFTPGO_VAULT)/g
EOF
}


set -e
main
