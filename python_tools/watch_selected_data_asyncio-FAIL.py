# -*- coding: utf-8 -*-
# watch_selected_data.py
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
import configparser
from datetime import datetime, timezone
from itertools import chain
import logging
import os
from pathlib import Path
from pprint import pformat
import ssl
from typing import TypeAlias

logger = logging.getLogger('watch_selected_data')

QueueType: TypeAlias = type(asyncio.Queue)

ACTIVE_CERTIFICATES_FILENAME = 'active_certificates.vars'
#DEFAULT_CONFIG_FILENAME = 'config.ini'



def read_vars_file(vars_file):
    """
    It is expected that the text file represented by vars_file
    is a "INI" file WITHOUT section headers.
    This function reads and parses this file with ConfigParser()
    by prepending "[DEFAULT]\n" to the beginning of the file.
    """
    # if not isinstance(vars_file, Path):
    #     vars_file = Path(vars_file)

    #see: https://stackoverflow.com/questions/2885190/using-configparser-to-read-a-file-without-section-name
    parser = configparser.ConfigParser()
    with open(vars_file) as lines:
        lines = chain(("[DEFAULT]",), lines)
        parser.read_file(lines)
        return parser['DEFAULT']


#-------------------------------------------------------------------------------
class WatchMqttMessages:
    """
    """

    def __init__(self, args):
        """
        args is the Command Line Arguments.
        Default config values are defined in this function.
        """
        self.args = args

        # self.config = configparser.ConfigParser()
        # self.config.read(self.args.active)

        # MQTT Connection Parameters:
        active_certificates = self.get_active_certificate_vars()
        certs_dir = self.get_active_certificates_dir(active_certificates)

        tls_params = aiomqtt.TLSParameters(
            ca_certs = certs_dir / 'mosq_ca.crt',
            certfile = certs_dir / 'client_a' / 'mosq_client.crt',
            keyfile  = certs_dir / 'client_a' / 'mosq_client.key',
            cert_reqs = ssl.CERT_REQUIRED,
            tls_version = ssl.PROTOCOL_TLSv1_2,
        )
        self.hostname = self.config.get('HOSTNAME')
        self.port = self.config.getint('PORT', fallback=8883)
        self.tls_params = tls_params
        self.protocol = aiomqtt.ProtocolVersion.V5
        #self.protocol = aiomqtt.ProtocolVersion.V311
        self.mqtt_topic = self.config.get('TOPIC', fallback='soilmoisture/#')


    def __repr__(self) -> str:
        tmp = {
            'hostname': self.hostname,
            'port': self.port,
            'tls_params': self.tls_params,
            'protocol': self.protocol,
            'mqtt_topic': self.mqtt_topic,
        }
        return pformat(tmp)


    def get_active_certificate_vars(self) -> str:
        """
        """
        # Check the config file for a certificate directory.
        # cert_dir = self.config.get('MQTT Connection', 'cert_dir', fallback='')
        # if cert_dir and os.path.isdir(cert_dir):
        #     return cert_dir

        # Try various directories named 'private'...
        common_dirs = (
            './private',
            '../private',
            '../../private',
            '~/private',
        )
        for dir_str in common_dirs:
            test_dir = Path(dir_str)
            logger.debug(f'private test_dir: {test_dir}')

            if test_dir.is_dir():
                vars_file = test_dir / self.args.active
                logger.debug(f'vars_file: {vars_file}')
                if vars_file.is_file():
                    self.config = read_vars_file(vars_file)

                    active_cert_vars = self.config.get('ACTIVE_CERTIFICATES_DIR', fallback='')
                    logger.debug(f'active_cert_vars: {active_cert_vars}')
                    if active_cert_vars:
                        return active_cert_vars

        raise Exception('Active Certificates NOT FOUND!')


    def get_active_certificates_dir(self, active_certificates) -> os.PathLike:
        """
        """
        # Try various directories named 'private'...
        common_dirs = (
            './certificates',
            '../certificates',
            '../../certificates',
            '~/certificates',
        )
        for dir_str in common_dirs:
            test_dir = Path(dir_str)
            logger.debug(f'certificates test_dir: {test_dir}')

            if test_dir.is_dir():
                certs_dir = test_dir / active_certificates
                logger.debug(f'certs_dir: {certs_dir}')
                return certs_dir

        raise Exception('Active Certificates Dir NOT FOUND!')


    def get_output_filename(self) -> os.PathLike:
        """
        Return a filename based on the current date.
        """
        now = datetime.now(timezone.utc)
        filename = now.strftime(self.output_filename_prefix + "%Y-%m-%d.csv")
        #filename = 'soilmoisture_2023-11-10.csv'
        return self.output_dir / filename


    def get_output_header(self) -> str:
        """
        Return the header row of the output file.
        This will be needed later when the csv file is processed.
        For this later processing, it is important to NOT have spaces around the commas because
          some systems take those spaces literally and use them in the data column name.
        """
        return 'sensor_id,sensor_value,utc_timestamp'


    async def listen_for_moisture_values(self, mqtt_listen_queue):
    #async def listen_for_moisture_values(self, mqtt_listen_queue: QueueType):
        """
        """
        logger.info(f'listen_for_moisture_values(...)\n{self}\n')

        async with aiomqtt.Client(
            hostname=self.hostname,
            port=self.port,
            tls_params=self.tls_params,
            protocol=self.protocol
        ) as client:
            async with client.messages() as messages:
                await client.subscribe(self.mqtt_topic)
                async for message in messages:
                    msg_str = '{},{}'.format(
                        message.topic.value,
                        message.payload.decode('utf-8')
                    )

                    # TODO: uncomment
                    #await mqtt_listen_queue.put(msg_str)
                    
                    logger.info('message: ' + msg_str)


    async def save_values_to_file(self, mqtt_listen_queue):
    #async def save_values_to_file(self, mqtt_listen_queue: QueueType):
        """
        """
        value_to_save = None

        while True:
            # asynchronously wait for the next queued value.
            logger.debug(f'waiting for queue...\n')
            value_to_save = await mqtt_listen_queue.get()
            logger.debug('got: ' + value_to_save)

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
        #mqtt_listen_queue = asyncio.Queue()
        mqtt_listen_queue: QueueType = asyncio.Queue()

        async with asyncio.TaskGroup() as task_group:
            task_group.create_task(self.listen_for_moisture_values(mqtt_listen_queue))
            #task_group.create_task(self.save_values_to_file(mqtt_listen_queue))

        #task1 = asyncio.create_task( self.listen_for_moisture_values(mqtt_listen_queue) )
        #task2 = asyncio.create_task( self.save_values_to_file(mqtt_listen_queue) )
        # ...
        #await task1
        #await task2


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
