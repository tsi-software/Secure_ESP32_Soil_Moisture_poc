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
from datetime import timedelta, datetime, tzinfo, timezone
import logging
import os
from pprint import pformat

import soilmoisture_config_utils as config_utils

logger = logging.getLogger('watch_selected_data_asyncio')
ACTIVE_CERTIFICATES_FILENAME = 'active_certificates.vars'


# Python >= 10
#from typing import TypeAlias
#QueueType: TypeAlias = type(asyncio.Queue)



#-------------------------------------------------------------------------------
class SoilMoistureTouchpadMessage:
    """
    Example MQTT 'topic payload':
    'soilmoisture/3f36213a-ec4b-43ea-8a85-ac6098fac883/touchpad/2 21086,1719948977'
    """
    def __init__(self, mqtt_message):
        logger.debug(repr(mqtt_message.topic))
        logger.debug(repr(mqtt_message.payload))

        self.valid = False
        self.mqtt_message = mqtt_message
        self.topic_parts = mqtt_message.topic.value.split('/')

        if 'soilmoisture' != self.topic_parts[0] or 'touchpad' != self.topic_parts[2]:
            # Not Valid.
            return

        self.controller_uuid = self.topic_parts[1]
        self.touchpad = int(self.topic_parts[3])

        touch_value, touch_timestamp = mqtt_message.payload.decode('ascii').split(',')
        self.touch_value = int(touch_value)
        self.touch_timestamp = int(touch_timestamp)
        self.touch_datetime = datetime.fromtimestamp(self.touch_timestamp, tz=timezone.utc).astimezone()
        self.touch_datetime_str = self.touch_datetime.strftime('%Y-%m-%d %H:%M:%S')

        self.message_str = f'Touch Pad: controller={self.controller_uuid}, touchpad={self.touchpad} value={self.touch_value}, {self.touch_datetime_str}'
        self.valid = True


    def __repr__(self) -> str:
        tmp = {
            'controller_uuid': self.controller_uuid,
            'touchpad': self.touchpad,
            'touch_value': self.touch_value,
            'touch_datetime_str': self.touch_datetime_str,
        }
        return pformat(tmp)


    def __str__(self) -> str:
        return  self.message_str


    def is_valid(self):
        return self.valid



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

                touchpad_msg = SoilMoistureTouchpadMessage(message)
                if touchpad_msg.is_valid():
                    logger.debug(f'put SoilMoistureTouchpadMessage {touchpad_msg}')
                    await mqtt_listen_queue.put(touchpad_msg)
                else:
                    msg_str = '{},{}'.format(
                        message.topic.value,
                        message.payload.decode('utf-8')
                    )
                    logger.debug(f'put generic message {msg_str}')
                    await mqtt_listen_queue.put(msg_str)


    async def save_values_to_file(self, mqtt_listen_queue):
    #async def save_values_to_file(self, mqtt_listen_queue: QueueType):
        """
        """
        while not self.cancelled:
            # asynchronously wait for the next queued value.
            logger.debug(f'waiting for queue...\n')
            queue_value = await mqtt_listen_queue.get()
            logger.debug(queue_value)
            print(queue_value)


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
