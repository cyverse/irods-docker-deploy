#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# This always exits with 0 status, as a work around for iRODS msiExecCmd microservice. To detect
# errors, monitor stderr.

import logging
import os
import time
import sys
import asyncio
from nats.aio.client import Client as NATS
from stan.aio.client import Client as STAN

async def main(loop):
    logging.basicConfig(stream=sys.stdout)

    natsURL = sys.argv[1]
    clusterID = sys.argv[2]
    clientID = sys.argv[3]
    
    nc = NATS()
    await nc.connect(natsURL, io_loop=loop)
    sc = STAN()
    await sc.connect(clusterID, clientID, nats=nc)

    total_messages = 0
    future = asyncio.Future(loop=loop)
    async def cb(msg):
        nonlocal future
        nonlocal total_messages
        print("Received a message (seq={}): {}".format(msg.seq, msg.data))
        total_messages += 1
        future.set_result(None)

    sub = await sc.subscribe("data-object.mv", start_at='first', cb=cb)
    await asyncio.wait_for(future, 1, loop=loop)

    await sub.unsubscribe()
    
    await sc.close()
    await nc.close()

if __name__ == '__main__':
    natsURL = sys.argv[1]
    if len(natsURL) > 0:
        loop = asyncio.get_event_loop()
        loop.run_until_complete(main(loop))
        loop.close()
