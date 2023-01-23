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
export IRODS_LAST_EPHEMERAL_PORT=20199
export IRODS_RES_CONF_NAME=localResc
export IRODS_SCHEMA_VALIDATION=off
export IRODS_VAULT=/var/lib/irods/Vault
export IRODS_ZONE_NAME=ucsb
export IRODS_ZONE_KEY=TEMPORARY_zone_key
export IRODS_CONTROL_PLANE_KEY=TEMPORARY__32byte_ctrl_plane_key
export IRODS_CONTROL_PLANE_PORT=1248
export IRODS_HOST=brain.ece.ucsb.edu
export IRODS_NEGOTIATION_KEY=TEMPORARY_32byte_negotiation_key
export IRODS_SYSTEM_GROUP=irods
export IRODS_SYSTEM_USER=irods
export IRODS_ZONE_PORT=1247
export IRODS_DEFAULT_RESOURCE="$IRODS_RES_CONF_NAME"
export IRODS_MAX_NUM_RE_PROCS=4

export IRODS_ADMIN_USER=rods
export IRODS_ADMIN_PASSWORD=test_rods_password
export IRODS_BISQUE_ADMIN_USER=bisque_admin
export IRODS_BISQUE_ADMIN_PASSWORD=test_bisque_admin_password

export DBMS_CONF_HOST="$ENV_NAME"_dbms_1."$DOMAIN"
export IRODS_CONF_HOST="$ENV_NAME"_irods_1."$DOMAIN"
export SFTPGO_CONF_HOST="$ENV_NAME"_sftpgo_1."$DOMAIN"
export RABBITMQ_CONF_HOST="$ENV_NAME"_rabbitmq_1."$DOMAIN"

export RABBITMQ_PORT=5672
export RABBITMQ_MANAGEMENT_PORT=15672
export RABBITMQ_ADMIN_USER=admin
export RABBITMQ_ADMIN_PASSWORD=test_admin_password
export RABBITMQ_IRODS_USER=irods
export RABBITMQ_IRODS_PASSWORD=test_irods_password
export RABBITMQ_IRODS_EXCHANGE=irods
export RABBITMQ_URL=amqp://$RABBITMQ_IRODS_USER:$RABBITMQ_IRODS_PASSWORD@$RABBITMQ_CONF_HOST:$RABBITMQ_PORT/

export WEBDAV_PORT=8080
export SFTP_PORT=2022
export SFTPGO_ADMIN_UI_PORT=8022
export SFTPGO_VAULT=/srv/sftpgo
export SFTPGO_HOME_PATH="$SFTPGO_VAULT/data"
export SFTPGO_ADMIN_USER=sftpgo_admin
export SFTPGO_ADMIN_PASSWORD=test_sftpgo_admin_password
export SFTPGO_LOG_DIR=/var/log/sftpgo

export IRODS_PROXY_USER="$IRODS_ADMIN_USER"
export IRODS_PROXY_PASSWORD="$IRODS_ADMIN_PASSWORD"
export IRODS_PORT="$IRODS_ZONE_PORT"
export IRODS_ZONE="$IRODS_ZONE_NAME"
export IRODS_SHARED=/"$IRODS_ZONE_NAME"/home/public

export BISQUE_URL=https://bisque.cyverse.org
export BISQUE_IRODS_BASE_URL=file:///irods_mount
export BISQUE_IRODS_ROOT_PATH=/"$IRODS_ZONE_NAME"/home
export BISQUE_ADMIN_USER=bisque_admin
export BISQUE_ADMIN_PASSWORD=test_bisque_admin_password

export DOCKER_IRODS_VOLUME=irods_volume
export DOCKER_DB_VOLUME=db_volume
export DOCKER_DB_BACKUP_VOLUME=db_backup_volume
export DOCKER_SFTPGO_VOLUME=sftpgo_volume
export LOCAL_IRODS_VOLUME_PATH=/opt/irods-docker-deploy/irods_volume
export LOCAL_DB_VOLUME_PATH=/opt/irods-docker-deploy/db_volume
export LOCAL_DB_BACKUP_VOLUME_PATH=/opt/irods-docker-deploy/db_backup_volume
export LOCAL_SFTPGO_VOLUME_PATH=/opt/irods-docker-deploy/sftpgo_volume