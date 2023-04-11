#!/bin/bash
#
# This script creates the sftpgo.json configuration file for SFTPGo service.
#
# It requires the following environment variables to be defined
#
# WEBDAV_PORT                     The TCP port for the webdav service
# SFTP_PORT                       The TCP port for the sftp service
# SFTPGO_ADMIN_UI_PORT            The TCP port for web admin service
# SFTPGO_VAULT                    The local path for storing user data and log
# SFTPGO_HOME_PATH                The local path for storing user home data
# IRODS_PROXY_USER                The proxy user for iRODS to provide authentication of users
# IRODS_PROXY_PASSWORD            The password of the proxy user for iRODS
# IRODS_HOST                      The iRODS hostname
# IRODS_PORT                      The iRODS port number
# IRODS_ZONE                      The iRODS zone name
# IRODS_SHARED                    The iRODS shared directory name (e.g., public / shared)


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
s/\$WEBDAV_PORT/$(escape $WEBDAV_PORT)/g
s/\$SFTP_PORT/$(escape $SFTP_PORT)/g
s/\$SFTPGO_ADMIN_UI_PORT/$(escape $SFTPGO_ADMIN_UI_PORT)/g
s/\$SFTPGO_VAULT/$(escape $SFTPGO_VAULT)/g
s/\$SFTPGO_HOME_PATH/$(escape $SFTPGO_HOME_PATH)/g
s/\$IRODS_PROXY_USER/$(escape $IRODS_PROXY_USER)/g
s/\$IRODS_PROXY_PASSWORD/$(escape $IRODS_PROXY_PASSWORD)/g
s/\$IRODS_HOST/$(escape $IRODS_HOST)/g
s/\$IRODS_PORT/$(escape $IRODS_PORT)/g
s/\$IRODS_ZONE/$(escape $IRODS_ZONE)/g
s/\$IRODS_SHARED/$(escape $IRODS_SHARED)/g
EOF
}


set -e
main
