#!/bin/bash
#
# This script configures a catalog provider.
#
# It requires the following environment variables to be defined.
#
# DB_NAME                     The name of the DB iRODS will use.
# DB_PASSWORD                 The password used to authenticate DB_USER within
#                             PostgreSQL.
# DB_USER                     The DBMS user iRODS will use to connect to DB_NAME
#                             DB.
# DBMS_HOST                   The FQDN or IP address of the PostgreSQL server
# DBMS_PORT                   The TCP port the PostgreSQL will listen on.
# IRODS_CONTROL_PLANE_KEY     The encryption key required for communicating with
#                             the grid control plane.
# IRODS_CONTROL_PLANE_PORT    The port on which the control plane operates.
# IRODS_DEFAULT_RESOURCE      The name of the default resource to use
# IRODS_FIRST_EPHEMERAL_PORT  The beginning of the port range available for
#                             parallel transfer and reconnections.
# IRODS_HOST                  The FQDN or IP address of the server being
#                             configured.
# IRODS_LAST_EPHEMERAL_PORT   The end of the port range available for parallel
#                             transfer and reconnections.
# IRODS_NEGOTIATION_KEY       The shared encryption key used by the zone in
#                             advanced negotiation handshake a the beginning of
#                             a client connection
# IRODS_SCHEMA_VALIDATION     The URI for the schema used to validate the
#                             configuration files or 'off'.
# IRODS_SYSTEM_GROUP          The system group for the iRODS process
# IRODS_SYSTEM_USER           The system user for the iRODS process
# IRODS_ZONE_KEY              The shared secred used for authentication during
#                             server-to-server communication
# IRODS_ZONE_NAME             The name of the iRODS zone.
# IRODS_ADMIN_PASSWORD         The password used to authenticate the
#                             IRODS_ADMIN_USER user.
# IRODS_ZONE_PORT             The main TCP port used by the zone for
#                             communication.
# IRODS_ADMIN_USER             The main rodsadmin user.

set -o errexit -o nounset -o pipefail

readonly CfgDir=/etc/irods
readonly HostAccessControlCfg="$CfgDir"/host_access_control_config.json
readonly HostsCfg="$CfgDir"/hosts_config.json
readonly ServerCfg="$CfgDir"/server_config.json
readonly SvcAccount="$CfgDir"/service_account.config

readonly HomeDir=/var/lib/irods
readonly Odbc="$HomeDir"/.odbc.ini
readonly Version="$HomeDir"/VERSION.json

readonly EnvDir="$HomeDir"/.irods
readonly EnvCfg="$EnvDir"/irods_environment.json


main()
{
  validate_32_byte_key "$IRODS_NEGOTIATION_KEY" "iRODS server's negotiation key"
  validate_32_byte_key "$IRODS_CONTROL_PLANE_KEY" 'Control Plane key'

  mk_svc_account
  mk_svc_account_cfg > "$SvcAccount"

  mk_server_cfg > "$ServerCfg"
  mk_host_access_control_cfg > "$HostAccessControlCfg"
  mk_hosts_cfg > "$HostsCfg"

  update_odbc_def 
  mk_odbc > "$Odbc"

  mk_version "$(date +%FT%T.000000)" > "$Version"

  mkdir --parents "$EnvDir"
  mk_irods_env > "$EnvCfg"

  ensure_ownership "$HomeDir"
  ensure_ownership "$CfgDir"
}


ensure_ownership()
{
  local fsEntity="$1"

  chown --recursive "$IRODS_SYSTEM_USER":"$IRODS_SYSTEM_GROUP" "$fsEntity"
  chmod --recursive u+rw,go= "$fsEntity"
}


# define service account for this installation
mk_svc_account()
{
  groupadd --force --system "$IRODS_SYSTEM_GROUP"

  local err
  if ! err="$(
    adduser --system --gid "$IRODS_SYSTEM_GROUP" --home-dir /var/lib/irods "$IRODS_SYSTEM_USER" \
      2>&1 )"
  then
    local rc=$?
    if (( rc != 9 ))
    then
      echo "$err" >&2
      return $rc
    fi
  fi
}


mk_host_access_control_cfg()
{
  cat /var/lib/irods/packaging/host_access_control_config.json.template
}


mk_hosts_cfg()
{
  cat /var/lib/irods/packaging/hosts_config.json.template
}


mk_irods_env()
{
  jq --sort-keys --from-file /dev/stdin <(echo '{}') <<JQ
.irods_host = "$IRODS_HOST" |
.irods_port = $IRODS_ZONE_PORT |
.irods_user_name = "$IRODS_ADMIN_USER" |
.irods_zone_name = "$IRODS_ZONE_NAME" |
.irods_client_server_negotiation = "request_server_negotiation" | 
.irods_client_server_policy = "CS_NEG_REFUSE" |
.irods_control_plane_key = "$IRODS_CONTROL_PLANE_KEY" |
.irods_control_plane_port = $IRODS_CONTROL_PLANE_PORT |
.irods_cwd = "/$IRODS_ZONE_NAME/home/$IRODS_ADMIN_USER" |
.irods_default_hash_scheme = "SHA256" |
.irods_default_resource = "$IRODS_DEFAULT_RESOURCE" |
.irods_encryption_algorithm = "AES-256-CBC" |
.irods_encryption_key_size = 32 |
.irods_encryption_num_hash_rounds = 16 |
.irods_encryption_salt_size = 8 |
.irods_home = "/$IRODS_ZONE_NAME/home/$IRODS_ADMIN_USER" |
.irods_match_hash_policy = "compatible" |
.schema_name = "service_account_environment" |
.schema_version = "v3"
JQ
}


mk_odbc()
{
  cat <<EOINI
[iRODS Catalog]
Driver=PostgreSQL
Description=iRODS Catalog
Trace=0
Database=$DB_NAME
TraceFile=
Servername=$DBMS_HOST
FakeOidIndex=No
ShowOidColumn=No
RowVersioning=No
CommLog=0
ReadOnly=no
Debug=0
ConnSettings=
Port=$DBMS_PORT
Ksqo=0
ShowSystemTables=No
EOINI
}


mk_server_cfg()
{
  jq --sort-keys --from-file /dev/stdin /var/lib/irods/packaging/server_config.json.template <<JQ
.catalog_provider_hosts |= [ "localhost" ] |
.catalog_service_role |= "provider" |
.default_resource_name |= "$IRODS_DEFAULT_RESOURCE" |
.negotiation_key |= "$IRODS_NEGOTIATION_KEY" |
.schema_validation_base_uri |= "$IRODS_SCHEMA_VALIDATION" |
.server_control_plane_key |= "$IRODS_CONTROL_PLANE_KEY" |
.server_control_plane_port |= $IRODS_CONTROL_PLANE_PORT |
.server_port_range_end |= $IRODS_LAST_EPHEMERAL_PORT |
.server_port_range_start |= $IRODS_FIRST_EPHEMERAL_PORT |
.zone_key |= "$IRODS_ZONE_KEY" |
.zone_name |= "$IRODS_ZONE_NAME" |
.zone_port |= $IRODS_ZONE_PORT |
.zone_user |= "$IRODS_ADMIN_USER" |
.plugin_configuration.database.postgres |= {
  "db_host": "$DBMS_HOST",
  "db_name": "$DB_NAME",
  "db_odbc_driver": "PostgreSQL",
  "db_password": "$DB_PASSWORD",
  "db_port": $DBMS_PORT,
  "db_username": "$DB_USER"
} |
# Add stub version of ipc-housekeeping contents so tests work
.plugin_configuration.rule_engines |= map_values(
  if .instance_name == "irods_rule_engine_plugin-irods_rule_language-instance" then 
    .plugin_specific_configuration.re_rulebase_set = 
      [ "pre-config" ] + .plugin_specific_configuration.re_rulebase_set  
  else 
    .
  end )
JQ
}


mk_svc_account_cfg()
{
  cat <<EOF
IRODS_SERVICE_ACCOUNT_NAME=$IRODS_SYSTEM_USER
IRODS_SERVICE_GROUP_NAME=$IRODS_SYSTEM_GROUP
EOF
}


mk_version()
{
  local installTime="$1"

  jq --sort-keys --from-file /dev/stdin /var/lib/irods/VERSION.json.dist <<JQ
.installation_time |= "$installTime" 
JQ
}

update_odbc_def()
{
  odbcinst -i -d -r -v <<EOF
[PostgreSQL]
Description = PostgreSQL 12 ODBC Driver
Driver = /usr/pgsql-12/lib/psqlodbc.so
Setup = /usr/pgsql-12/lib/psqlodbcw.so
EOF
}


validate_32_byte_key()
{
  local keyVal="$1"
  local keyName="$2"

  # check length (must equal 32)
  if [ ${#keyVal} -ne 32 ]
  then
    printf '%s needs to be 32 bytes long\n' "$keyName" >&2
    return 1
  fi
}


main
