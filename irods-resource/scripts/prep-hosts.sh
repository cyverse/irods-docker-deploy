#!/bin/bash
#
# This script adds icat hosts to /etc/hosts.
#
# IRODS_CONF_HOST          The hostname of iRODS server that runs iCAT in the docker network.
# IRODS_HOST               The hostname of iRODS server that runs iCAT.

IRODS_HOST_IP=$(dig +short $IRODS_HOST)

main()
{ 
  echo "\n$IRODS_HOST_IP  $IRODS_CONF_HOST\n" >> /etc/hosts
}


set -e
main
