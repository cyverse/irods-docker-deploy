#!/usr/bin/env python
# -*- coding: utf-8 -*-

""" Script to register irods files with bisque (code checked with Pycharm 2018.x) """

# +
# import(s)
# -
from __future__ import print_function
import argparse
import os
import six
import sys
import logging
import requests

# try:
#     # noinspection PyPep8Naming
#     from lxml import etree as ElementTree
# except ImportError:
#     import xml.etree.ElementTree as ElementTree

import xml.etree.ElementTree as ElementTree
from xml.etree.ElementTree import ParseError

# noinspection PyBroadException
try:
    # python 2
    from ConfigParser import SafeConfigParser
except:
    # python 3
    from configparser import SafeConfigParser


# +
# dunder(s)
# -
__author__ = "Center for Bioimage Informatics"
__version__ = "1.1"
__copyright__ = "Center for BioImage Informatics, University California, Santa Barbara"


# +
# constant(s)
#
# Config for local installation - values can be stored in a file ~/.bisque or /etc/bisque/bisque_config with format:
#   [bqpath]
#   bisque_admin_user = admin
#   bisque_admin_pass = gobbledygook
# -
DEFAULTS = dict(
    logfile='/tmp/bisque_paths.log',
    bisque_host='https://bisque.cyverse.org',
    bisque_admin_user='admin',
    bisque_admin_pass='********',
    irods_host='irods://data.iplantcollaborative.org',
    )


# +
# function: bqpath_get_environment()
# -
def bqpath_get_environment():
    config = SafeConfigParser()
    config.add_section('bqpath')
    for k, v in DEFAULTS.items():
        config.set('bqpath', k, v)
    config.read(['.bisque', os.path.expanduser('~/.bisque'), '/etc/bisque/bisque_config'])
    return dict(config.items('bqpath'))


# +
# function: bqpath_connect()
# -
def bqpath_connect(benv=None, log=None, usr='', pwd=''):
    """
        :param benv: input connection dictionary
        :param log: input logger
        :param usr: input username (if nothing in dictionary)
        :param pwd: input password (if nothing in dictionary)
        :return: connection object to irods using credentials dictionary or None
    """

    # get input(s)
    if benv is None or not isinstance(benv, dict) or not benv:
        return None
    else:
        # noinspection PyBroadException
        try:
            credentials = '{}:{}'.format(benv['bisque_admin_user'], benv['bisque_admin_pass'])
        except:
            if usr != '' and pwd != '':
                credentials = '{}:{}'.format(usr, pwd)
            else:
                return None
    if log is not None:
        log.debug('Using credentials={}'.format(credentials))

    # connect and perform request
    try:
        this_session = requests.Session()
        this_session.log = logging.getLogger('rods2bq')
        this_session.auth = tuple(credentials.split(':'))
    except requests.exceptions.HTTPError as e:
        this_session = None
        if log is not None:
            log.exception("exception occurred %s : %s", e, e.response.text)
        six.print_("ERROR:", e.response and e.response.status_code)

    # return session
    return this_session


# +
# function: resource_element()
# -
def resource_element(args):
    """ check the args and create a compatible resource element for posting or linking """

    # load file into resource
    if args.tag_file:
        try:
            resource = ElementTree.parse(args.tag_file).getroot()
        except ParseError as pe:
            six.print_('Parse failure: aborting: ', pe)
            return
    else:
        resource = ElementTree.Element(args.resource or 'resource')

    # set resource(s)
    for fld in ('permission', 'hidden'):
        if getattr(args, fld) is not None:
            resource.set(fld, getattr(args, fld))

    if args.srcpath:
        resource.set('value', args.srcpath[0])
        resource.set('name', os.path.basename(args.srcpath[0]))

    # return resource
    return resource


# +
# function: bisque_delete()
# -
def bisque_delete(session, args):
    """ delete a file based on the irods path """
    session.log.info("delete %s", args)

    url = args.host + "/blob_service/paths/remove"
    params = dict(path=args.srcpath)
    if args.alias:
        params['user'] = args.alias

    r = session.get(url, params=params)
    if r.status_code == requests.codes.ok:
        six.print_(r.text)

    r.raise_for_status()


# +
# function: bisque_link()
# -
def bisque_link(session, args):
    """ link a file based on the irods path """
    session.log.info("link %s", args)

    url = args.host + "/blob_service/paths/insert"
    params = {}
    resource = resource_element(args)
    payload = ElementTree.tostring(resource)
    if args.alias:
        params['user'] = args.alias

    session.log.info("url=%s", url)
    session.log.info("params=%s", params)
    session.log.info("payload=%s", payload)
    session.log.info("link (again) %s", args)

    r = session.post(url, data=payload, params=params, headers={'content-type': 'application/xml'})
    if r.status_code == requests.codes.ok:
        if args.compatible:
            response = ElementTree.fromstring(r.text)
            uniq = response.get('resource_uniq')
            uri = response.get('uri')
            six.print_(uniq, uri)
        else:
            six.print_(r.text)

    r.raise_for_status()


# +
# function: bisque_copy()
# -
def bisque_copy(session, args):
    """ copy a file based on the irods path """
    session.log.info("insert %s", args)

    url = args.host + "/import/transfer"
    params = {}
    resource = resource_element(args)
    del resource.attrib['value']

    files = {'file': (os.path.basename(args.srcpath[0]), open(args.srcpath[0], 'rb')),
             'file_resource': (None, ElementTree.tostring(resource), 'text/xml')}

    if args.alias:
        params['user'] = args.alias

    r = session.post(url, files=files, params=params)
    if r.status_code == requests.codes.ok:
        six.print_(r.text)

    r.raise_for_status()


# +
# function: bisque_rename()
# -
def bisque_rename(session, args):
    """ rename based on paths """
    session.log.info("rename %s", args)

    url = args.host + "/blob_service/paths/move"
    params = {'path': args.srcpath[0], 'destination': args.dstpath}
    if args.alias:
        params['user'] = args.alias

    r = session.get(url, params=params)
    if r.status_code == requests.codes.ok:
        six.print_(r.text)

    r.raise_for_status()


# +
# function: bisque_list()
# -
def bisque_list(session, args):
    """ list a file based on the irods path """
    session.log.info("list %s", args)

    url = args.host + "/blob_service/paths/list"

    # PND 07/19/2017: new code added to support --cyverse and --irods flags
    params = {}
    if len(args.srcpath) > 0:
        if args.cyverse:
            new_path = []
            if not args.srcpath[0].startswith('irods://'):
                new_path.append('irods://data.cyverse.org' + args.srcpath[0])
                args.srcpath = new_path
        elif args.irods:
            new_path = []
            if not args.srcpath[0].startswith('irods://'):
                new_path.append('irods://data.iplantcollaborative.org' + args.srcpath[0])
                args.srcpath = new_path
        # original code
        params = {'path': args.srcpath[0]}

    if args.alias:
        params['user'] = args.alias

    session.log.info("url=%s", url)
    session.log.info("params=%s", params)
    session.log.info("list (again) %s", args)

    r = session.get(url, params=params)

    if r.status_code == requests.codes.ok:
        if args.compatible:
            for resource in ElementTree.fromstring(r.text):
                six.print_(resource.get('resource_uniq'))
            return
        else:
            if args.unique:
                for resource in ElementTree.fromstring(r.text):
                    six.print_(resource.get('resource_uniq'), resource.get('resource_value'))
                return
        six.print_(r.text)

    r.raise_for_status()


# +
# data structure(s)
# -
OPERATIONS = {
    'ls': bisque_list,
    'ln': bisque_link,
    'cp': bisque_copy,
    'mv': bisque_rename,
    'rm': bisque_delete,
}

DESCRIPTION = """

Manipulate bisque resource with paths

insert, link, move and remove resource by their path.

"""


# +
# function: main()
# -
def main():

    # get environment
    benv = bqpath_get_environment()

    # add an argument parser
    parser = argparse.ArgumentParser(description=DESCRIPTION, formatter_class=argparse.ArgumentDefaultsHelpFormatter,)
    parser.add_argument('--alias', help="do action on behalf of user specified")
    parser.add_argument('-d', '--debug', action="store_true", default=False, help="log debugging")
    parser.add_argument('-H', '--host', default=benv['bisque_host'], help="bisque host")
    parser.add_argument('-c', '--credentials',
                        default="%s:%s" % (benv['bisque_admin_user'], benv["bisque_admin_pass"]),
                        help="user credentials")
    parser.add_argument('-C', '--compatible',  action="store_true", help="Make compatible with old script")
    parser.add_argument('-V', '--verbose', action='store_true', help='print stuff')

    # add a sub-parser stub
    sp = parser.add_subparsers()

    # ls sub-parser
    lsp = sp.add_parser('ls')
    lsp.add_argument('-u', '--unique', default=None, action="store_true", help="return unique codes")
    lspe = lsp.add_mutually_exclusive_group()
    lspe.add_argument('-y', '--cyverse', default=None, action="store_true",
                      help="prepend path with irods://data.cyverse.org")
    lspe.add_argument('-i', '--irods', default=None, action="store_true",
                      help="prepend path with irods://data.iplantcollaborative.org")
    lsp.add_argument('paths', nargs='+')
    lsp.set_defaults(func=bisque_list)

    # ln sub-parser
    lnp = sp.add_parser('ln')
    lnp.add_argument('-T', '--tag_file', default=None, help="tag document for insert")
    lnp.add_argument('-P', '--permission', default="private", help="Set resource permission (compatibility)")
    lnp.add_argument('-R', '--resource', default=None, help='force resource type')
    lnp.add_argument('--hidden', default=None, help="Set resource visibility (hidden)")
    lnp.add_argument('paths', nargs='+')
    lnp.set_defaults(func=bisque_link)

    # cp sub-parser
    cpp = sp.add_parser('cp')
    cpp.add_argument('paths',  nargs='+')
    cpp.add_argument('-T', '--tag_file', default=None, help="tag document for insert")
    cpp.add_argument('-P', '--permission', default="private", help="Set resource permission (compatibility)")
    cpp.add_argument('--hidden', default=None, help="Set resource visibility (hidden)")
    cpp.set_defaults(func=bisque_copy)

    # mv sub-parser
    mvp = sp.add_parser('mv')
    mvp.add_argument('paths',  nargs='+')
    mvp.set_defaults(func=bisque_rename)

    # rm sub-parser
    rmp = sp.add_parser('rm')
    rmp.add_argument('paths',  nargs='+')
    rmp.set_defaults(func=bisque_delete)

    # get a logger
    logging.basicConfig(filename=benv['logfile'], level=logging.INFO,
                        format="%(asctime)s %(levelname)-5.5s [%(name)s] %(message)s")
    log = logging.getLogger('rods2bq')

    # parse command line
    args = parser.parse_args()

    # -d, --debug
    if args.debug:
        logging.getLogger().setLevel(logging.DEBUG)

    # path(s)
    if len(args.paths) > 1:
        args.dstpath = args.paths.pop()
        args.srcpath = args.paths
    else:
        args.srcpath = args.paths

    # -C, --compatible
    if args.compatible:
        paths = []
        irods_host = benv.get('irods_host')
        for el in args.srcpath:
            if not el.startswith('irods://'):
                paths.append(irods_host + el)
            else:
                paths.append(el)
        args.srcpath = paths
        if args.dstpath and not args.dstpath.startswith('irods://'):
            args.dstpath = irods_host + args.dstpath

    # output a message
    if args.debug:
        six.print_(args, file=sys.stderr)

    # connect and perform request
    session = bqpath_connect(benv, log)
    args.func(session, args)


# +
# entry point
# -
if __name__ == "__main__":
    main()
