#!/bin/bash
#
# This script creates the configuration file for irods-rule-async-exec-cmd service.
#
# It requires the following environment variables to be defined
#
# RABBITMQ_URL              The URL of RabbitMQ
# RABBITMQ_IRODS_EXCHANGE   The name of RabbitMQ exchange for iRODS messages
# BISQUE_URL                The BisQue Service URL, https://bisque.ucsb.edu
# BISQUE_ADMIN_USER         The BisQue admin username 
# BISQUE_ADMIN_PASSWORD     The BisQue admin user password
# BISQUE_IRODS_BASE_URL     The iRODS base URL for BisQue service, file:// or irods://
# BISQUE_IRODS_ROOT_PATH    The iRODS root path mounted to BisQue, "/" or "/iplant/home"
# IRODS_HOST                The FQDN or IP address of the server being configured.
# IRODS_ZONE_PORT           The main TCP port used by the zone for communication                           communication.
# IRODS_ZONE_NAME           The name of the iRODS zone.
# IRODS_ADMIN_USER          The main rodsadmin user.
# IRODS_ADMIN_PASSWORD      The password used to authenticate the IRODS_ADMIN_USER user.
# IRODS_BISQUE_ADMIN_USER   The iRODS username for BisQue


main()
{
  mkdir -p /etc/irods_rule_async_exec_cmd
  expand_tmpl > /etc/irods_rule_async_exec_cmd/config.yml
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
  cat <<EOF | sed --file - /tmp/rule_async_exec_cmd_config.yml.template
s/\$RABBITMQ_URL/$(escape $RABBITMQ_URL)/g
s/\$RABBITMQ_IRODS_EXCHANGE/$(escape $RABBITMQ_IRODS_EXCHANGE)/g
s/\$BISQUE_URL/$(escape $BISQUE_URL)/g
s/\$BISQUE_ADMIN_USER/$(escape $BISQUE_ADMIN_USER)/g
s/\$BISQUE_ADMIN_PASSWORD/$(escape $BISQUE_ADMIN_PASSWORD)/g
s/\$BISQUE_IRODS_BASE_URL/$(escape $BISQUE_IRODS_BASE_URL)/g
s/\$BISQUE_IRODS_ROOT_PATH/$(escape $BISQUE_IRODS_ROOT_PATH)/g
s/\$IRODS_HOST/$(escape $IRODS_HOST)/g
s/\$IRODS_ZONE_PORT/$(escape $IRODS_ZONE_PORT)/g
s/\$IRODS_ZONE_NAME/$(escape $IRODS_ZONE_NAME)/g
s/\$IRODS_ADMIN_USER/$(escape $IRODS_ADMIN_USER)/g
s/\$IRODS_ADMIN_PASSWORD/$(escape $IRODS_ADMIN_PASSWORD)/g
s/\$IRODS_BISQUE_ADMIN_USER/$(escape $IRODS_BISQUE_ADMIN_USER)/g
EOF
}


set -e
main
