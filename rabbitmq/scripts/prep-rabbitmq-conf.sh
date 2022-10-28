#!/bin/bash
#
# This script creates the rabbitmq.conf configuration file for RabbitMQ service.
#
# It requires the following environment variables to be defined
#
# RABBITMQ_PORT                       The TCP port for the RabbitMQ service
# RABBITMQ_MANAGEMENT_PORT            The TCP port for the RabbitMQ web admin service


main()
{
  expand_tmpl > /etc/rabbitmq/rabbitmq.conf
  chmod a+r /etc/rabbitmq/rabbitmq.conf
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
  cat <<EOF | sed --file - /tmp/rabbitmq.conf.template
s/\$RABBITMQ_PORT/$(escape $RABBITMQ_PORT)/g
s/\$RABBITMQ_MANAGEMENT_PORT/$(escape $RABBITMQ_MANAGEMENT_PORT)/g
EOF
}


set -e
main
