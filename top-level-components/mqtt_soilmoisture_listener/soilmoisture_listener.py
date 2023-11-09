# -*- coding: utf-8 -*-
# soilmoisture_listener.py
# Written with python 3.11 features.
#
import asyncio
import aiomqtt
# from aiomqtt import Client, ProtocolVersion, TLSParameters, Topic, Wildcard, Will
# from aiomqtt.error import MqttError, MqttReentrantError
# from aiomqtt.types import PayloadType
import argparse
import logging
import os
from pprint import pformat

logger = logging.getLogger('soilmoisture_listener')



#-------------------------------------------------------------------------------
def get_cert_dir(args) -> str:
    """
    """
    if args.certdir and os.path.isdir(args.certdir):
        return args.certdir

    common_dirs = (
        './private',
        '../private',
        '../../private',
        '~/private',
    )
    for test_dir in common_dirs:
        if os.path.isdir(test_dir):
            return test_dir

    raise Exception('Certificate Directory NOT FOUND!')



def get_config_dir(args) -> str:
    """
    """
    return '.'



def get_config(args) -> dict[str, str | int]:
    """
    """
    config_dir: str = get_config_dir(args)
    cert_dir: str = get_cert_dir(args)

    tls = {
        'ca_certs': cert_dir + '/mosq_ca.crt',
        'certfile': cert_dir + '/mosq_client.crt',
        'keyfile': cert_dir + '/mosq_client.key',
    }

    return {
        'hostname': '',
        'port': 8333,
        'tls': tls,
    }



async def listen():
    """
    """




async def start(args, config):
    """
    """
    logger.debug('start()')

    async with asyncio.TaskGroup() as task_group:
        task_group.create_task(listen())
        #task_group.create_task(sleep(2)) # any other generic task...



#-------------------------------------------------------------------------------
def commandLineArgs():
    parser = argparse.ArgumentParser()
    parser.add_argument("-c", "--certdir", help="Mosquitto security certificates directory.")
    parser.add_argument("-d", "--debug", action="store_true", help="Run in debug mode.")
    return parser.parse_args()



def main(args) -> None:
    """
    """
    config = get_config(args)
    logger.info('config: {}'.format(pformat(config)))

    if args.debug:
        asyncio.run(start(args, config), debug=True)
    else:
        asyncio.run(start(args, config))



#-------------------------------------------------------------------------------
logger.debug('name: {}'.format(__name__))
if __name__ == "__main__":
    args = commandLineArgs()
    if args.debug:
        logging.basicConfig(level=logging.DEBUG)
    else:
        logging.basicConfig(level=logging.INFO)
    main(args)
