#!/bin/bash
#
# This script creates the definitions.json configuration file for RabbitMQ service.
#
# It requires the following environment variables to be defined
#
# RABBITMQ_ADMIN_USER             The admin user account for the RabbitMQ service
# RABBITMQ_ADMIN_PASSWORD         The admin user password for the RabbitMQ service
# RABBITMQ_IRODS_USER             The irods user account for the RabbitMQ service
# RABBITMQ_IRODS_PASSWORD         The irods user password for the RabbitMQ service
# RABBITMQ_IRODS_EXCHANGE         The exchange for the RabbitMQ service

main()
{
  expand_tmpl > /etc/rabbitmq/definitions.json
  chmod a+r /etc/rabbitmq/definitions.json
}


# escapes / and \ for sed script
escape()
{
  local var="$*"

  # Escape \ first to avoid escaping the escape character, i.e. avoid / -> \/ -> \\/
  var="${var//\\/\\\\}"

  printf '%s' "${var//\//\\/}"
}


password_hash()
{
  local pass=$(gen_password.py "$*")

  printf '%s' $(escape "$pass")
}


expand_tmpl()
{
  cat <<EOF | sed --file - /tmp/definitions.json.template
s/\$RABBITMQ_ADMIN_USER/$(escape $RABBITMQ_ADMIN_USER)/g
s/\$RABBITMQ_ADMIN_PASSWORD_HASH/$(password_hash $RABBITMQ_ADMIN_PASSWORD)/g
s/\$RABBITMQ_IRODS_USER/$(escape $RABBITMQ_IRODS_USER)/g
s/\$RABBITMQ_IRODS_PASSWORD_HASH/$(password_hash $RABBITMQ_IRODS_PASSWORD)/g
s/\$RABBITMQ_IRODS_EXCHANGE/$(escape $RABBITMQ_IRODS_EXCHANGE)/g
EOF
}


set -e
main
