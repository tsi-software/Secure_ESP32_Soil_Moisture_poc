# -*- coding: utf-8 -*-
# watch_selected_data_asyncio.py
# Written for python 3.11
#
# Setup:
# python3 -m venv .venv
# source .venv/bin/activate
# pip3 install --upgrade pip
# pip3 install -r requirements.txt
#
import asyncio
import aiofiles
import aiomqtt
import argparse
#import configparser
from datetime import timedelta, datetime, tzinfo, timezone
#from itertools import chain
import logging
import os
#from pathlib import Path
from pprint import pformat
#import ssl

import soilmoisture_config_utils as config_utils

logger = logging.getLogger('watch_selected_data_asyncio')
ACTIVE_CERTIFICATES_FILENAME = 'active_certificates.vars'


# Python >= 10
#from typing import TypeAlias
#QueueType: TypeAlias = type(asyncio.Queue)



#-------------------------------------------------------------------------------
class WatchMqttMessages:
    """
    """

    def __init__(self, args):
        """
        args is the Command Line Arguments.
        Default config values are defined in this function.
        """
        self.cancelled = False
        self.args = args
        self.config_vars = config_utils.read_config_file(self.args)
        self.active_certificates = config_utils.get_active_certificates_vars(self.config_vars)
        self.certs_dir = config_utils.get_active_certificates_dir(self.active_certificates)
        self.connection_parameters = config_utils.get_connection_parameters(
            self.config_vars, self.certs_dir, mqtt_protocol=aiomqtt.ProtocolVersion.V5
        )

        tls_params = aiomqtt.TLSParameters(
            ca_certs = self.connection_parameters['tls_params']['ca_certs'],
            certfile = self.connection_parameters['tls_params']['certfile'],
            keyfile = self.connection_parameters['tls_params']['keyfile'],
            cert_reqs = self.connection_parameters['tls_params']['cert_reqs'],
            tls_version = self.connection_parameters['tls_params']['tls_version'],
        )

        self.hostname = self.connection_parameters['hostname']
        self.port = self.connection_parameters['port']
        self.tls_params = tls_params
        self.protocol = self.connection_parameters['protocol']
        self.mqtt_topic = self.connection_parameters['mqtt_topic']


    def __repr__(self) -> str:
        tmp = {
            'hostname': self.hostname,
            'port': self.port,
            'tls_params': self.tls_params,
            'protocol': self.protocol,
            'mqtt_topic': self.mqtt_topic,
        }
        return pformat(tmp)


    async def listen_for_moisture_values(self, mqtt_listen_queue):
    #async def listen_for_moisture_values(self, mqtt_listen_queue: QueueType):
        """
        """
        logger.info(f'listen_for_moisture_values(...)\n{self}\n')

        # see:
        # https://sbtinstruments.github.io/aiomqtt/subscribing-to-a-topic.html
        async with aiomqtt.Client(
            hostname=self.hostname,
            port=self.port,
            tls_params=self.tls_params,
            protocol=self.protocol
        ) as client:
            if self.cancelled:
                raise asyncio.CancelledError()

            await client.subscribe(self.mqtt_topic)
            async for message in client.messages:
                if self.cancelled:
                    raise asyncio.CancelledError()

                msg_str = '{},{}'.format(
                    message.topic.value,
                    message.payload.decode('utf-8')
                )
                logger.debug('put message: ' + msg_str)
                await mqtt_listen_queue.put(msg_str)


    async def save_values_to_file(self, mqtt_listen_queue):
    #async def save_values_to_file(self, mqtt_listen_queue: QueueType):
        """
        """
        value_to_save = None

        while not self.cancelled:
            # asynchronously wait for the next queued value.
            logger.debug(f'waiting for queue...\n')
            value_to_save = await mqtt_listen_queue.get()
            logger.info('message: ' + value_to_save)

            try:
                pass

            except TimeoutError:
                # Ignore TimeoutErrors here.
                # The intentional side effects are:
                #  1) Close and save the output file because no values are being processed at the moment
                #     and the file is not needed.
                #  2) Go back to the top of the outside loop where we 'async' wait for the next queued value.
                #  3) Because the output file has been closed it will need to be re-open,
                #     this gives us the opportunity to use a new filename based on the current date
                #     if the date has just changed.
                logger.debug('file flushed and closed.')
                pass
            except Exception as err:
                logger.error(f"Unexpected {err=}, {type(err)=}")
                raise


    async def start(self):
        """
        """
        logger.debug(f'start()\n{self}')
        mqtt_listen_queue = asyncio.Queue()
        #mqtt_listen_queue: QueueType = asyncio.Queue()

        # Python < 11
        try:
            await asyncio.gather(
                self.listen_for_moisture_values(mqtt_listen_queue),
                self.save_values_to_file(mqtt_listen_queue),
            )
        finally:
            self.cancelled = True
        #
        # listen_task = asyncio.create_task(self.listen_for_moisture_values(mqtt_listen_queue))
        # save_task = asyncio.create_task(self.save_values_to_file(mqtt_listen_queue))
        # await listen_task
        # await save_task

        # Python >= 11
        # async with asyncio.TaskGroup() as task_group:
        #     task_group.create_task(self.listen_for_moisture_values(mqtt_listen_queue))
        #     task_group.create_task(self.save_values_to_file(mqtt_listen_queue))


#-------------------------------------------------------------------------------
def commandLineArgs():
    parser = argparse.ArgumentParser(
            description='Listen for, and display, MQTT messages with the Topic "soilmoisture/#"'
        )
    parser.add_argument("--active", default=ACTIVE_CERTIFICATES_FILENAME,  help="Active Certificates Filename. (default: %(default)s)")
    #parser.add_argument("--config", default=DEFAULT_CONFIG_FILENAME,  help="Configuration filename. (default: %(default)s)")
    parser.add_argument("--debug", action="store_true", help="Run in debug mode.")
    return parser.parse_args()


def main(args) -> None:
    """
    """
    watch_mqtt_messages = WatchMqttMessages(args)

    if args.debug:
        asyncio.run(watch_mqtt_messages.start(), debug=True)
    else:
        asyncio.run(watch_mqtt_messages.start())



#-------------------------------------------------------------------------------
logger.debug('name: {}'.format(__name__))
if __name__ == "__main__":
    args = commandLineArgs()
    if args.debug:
        logging.basicConfig(level=logging.DEBUG)
    else:
        logging.basicConfig(level=logging.INFO)
    main(args)
