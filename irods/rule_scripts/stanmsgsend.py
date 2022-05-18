#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# This always exits with 0 status, as a work around for iRODS msiExecCmd microservice. To detect
# errors, monitor stderr.

import logging
import os
import sys
import asyncio
from nats.aio.client import Client as NATS
from stan.aio.client import Client as STAN

async def main(loop):
    logging.basicConfig(stream=sys.stdout)

    natsURL = sys.argv[1]
    clusterID = sys.argv[2]
    clientID = sys.argv[3]
    key = sys.argv[4]
    body = sys.argv[5]

    nc = NATS()
    await nc.connect(natsURL, io_loop=loop)
    sc = STAN()
    await sc.connect(clusterID, clientID, nats=nc)

    await sc.publish(key, str.encode(body))

    await sc.close()
    await nc.close()

    #with open("/tmp/irods_message_out.txt", "a") as f:
    #    f.write("%s\t%s\t%s\t%s\t%s" % (natsURL, clusterID, clientID, key, body))

if __name__ == '__main__':
    natsURL = sys.argv[1]
    if len(natsURL) > 0:
        loop = asyncio.get_event_loop()
        loop.run_until_complete(main(loop))
        loop.close()
