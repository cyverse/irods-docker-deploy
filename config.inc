#!/bin/bash

export ENV_NAME=ucsb_irods
export DOMAIN="$ENV_NAME"_default

export DBMS_HOST="$ENV_NAME"_dbms_1."$DOMAIN"
export DBMS_TYPE=postgres
export DBMS_PORT=5432

export DB_NAME=ICAT
export DB_USER=irods
export DB_PASSWORD=test_db_password

export IRODS_FIRST_EPHEMERAL_PORT=20000
export IRODS_LAST_EPHEMERAL_PORT=20009
export IRODS_RES_CONF_NAME=localRes
export IRODS_SCHEMA_VALIDATION=off
export IRODS_VAULT=/var/lib/irods/Vault
export IRODS_ZONE_NAME=ucsb
export IRODS_ZONE_KEY=TEMPORARY_zone_key
export IRODS_CONTROL_PLANE_KEY=TEMPORARY__32byte_ctrl_plane_key
export IRODS_CONTROL_PLANE_PORT=1248
export IRODS_HOST=ies
export IRODS_NEGOTIATION_KEY=TEMPORARY_32byte_negotiation_key
export IRODS_SYSTEM_GROUP=irods
export IRODS_SYSTEM_USER=irods
export IRODS_ZONE_PORT=1247
export IRODS_DEFAULT_RESOURCE="$IRODS_RES_CONF_NAME"

export IRODS_ADMIN_USER=rods
export IRODS_ADMIN_PASSWORD=test_rods_password

#export IRODS_CONSUMER_CONF_HOST="$ENV_NAME"_consumer_1."$DOMAIN"
export IRODS_CONF_HOST="$ENV_NAME"_irods_1."$DOMAIN"

