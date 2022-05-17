#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# This always exits with 0 status, as a work around for iRODS msiExecCmd microservice. To detect
# errors, monitor stderr.

import logging
#import pika
import os
import sys

try:
    logging.basicConfig(stream=sys.stdout)

    exchange = sys.argv[1]
    key = sys.argv[2]
    body = sys.argv[3]

    """
    connParams = pika.URLParameters(os.environ['IRODS_AMQP_URI'])

    connection = pika.BlockingConnection(connParams)
    channel = connection.channel()

    channel.basic_publish(exchange=exchange,
                          routing_key=key,
                          body=body,
                          properties=pika.BasicProperties(delivery_mode=2))

    connection.close()
    """

    with open("/tmp/irods_message_out.txt", "a") as f:
        f.write("%s\t%s\t%s" % (exchange, key, body))

except BaseException as e:
    sys.stderr.write('ERROR: %s\n' % e)
    sys.exit(1)

