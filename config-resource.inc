#!/bin/bash

. config.inc
export ENV_NAME=ucsb_irods
export DOMAIN="$ENV_NAME"_default
export IRODS_RESOURCE_CONF_HOST="$ENV_NAME"_irods_resc_1."$DOMAIN"

export IRODS_RESOURCE_HOST=synology.ece.ucsb.edu

# override this with physical one
export RABBITMQ_URL=amqp://$RABBITMQ_IRODS_USER:$RABBITMQ_IRODS_PASSWORD@$IRODS_HOST:$RABBITMQ_PORT/
export IRODS_DEFAULT_RESOURCE=sharedResc

export DOCKER_IRODS_RESOURCE_VOLUME=irods_resource_volume
export LOCAL_IRODS_RESOURCE_VOLUME_PATH=/opt/irods-docker-deploy/irods_resource_volume