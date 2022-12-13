#!/bin/bash
#
# Execute irods-rule-async-exec-cmd.
#
# USAGE:
#  irods-rule-async-exec-cmd subcmd args...
#
# PARAMETERS:
#  subcmd  Either "send_msg" or "bisque_link"

/usr/local/bin/irods-rule-async-exec-cmd "$@"
