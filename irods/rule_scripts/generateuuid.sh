#!/bin/bash -
#
# This script generates a UUID. For Linux systems, this will be a time-based UUID. To guarantee 
# uniqueness among concurrent executions of this script, the uuidd daemon should be running.

if [ "$(uname)" == Linux ]
then
    uuidgen -t
else
    uuidgen | tr '[:upper:]' '[:lower:]'
fi
