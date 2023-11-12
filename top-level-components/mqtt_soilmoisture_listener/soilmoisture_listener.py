# -*- coding: utf-8 -*-
# soilmoisture_listener.py
# Written with python 3.11 features.
#
import asyncio
import aiofiles
import aiomqtt
import argparse
import configparser
from datetime import datetime
import logging
import os
from pathlib import Path
from pprint import pformat
import ssl
from typing import TypeAlias

logger = logging.getLogger('soilmoisture_listener')

QueueType: TypeAlias = type(asyncio.Queue)

DEFAULT_CONFIG_FILENAME = 'config.ini'



#-------------------------------------------------------------------------------
class SaveMqttMessages:
    """
    """

    def __init__(self, args):
        """
        args is the Command Line Arguments.
        """
        self.args = args

        self.config = configparser.ConfigParser()
        self.config.read([DEFAULT_CONFIG_FILENAME, self.args.config])

        # MQTT Connection Parameters:
        cert_dir: os.PathLike = Path(self.get_cert_dir())
        tls_params = aiomqtt.TLSParameters(
            ca_certs = cert_dir / 'mosq_ca.crt',
            certfile = cert_dir / 'mosq_client.crt',
            keyfile  = cert_dir / 'mosq_client.key',
            cert_reqs = ssl.CERT_REQUIRED,
            tls_version = ssl.PROTOCOL_TLSv1_2,
        )
        self.hostname = self.config.get('MQTT Connection', 'hostname')
        self.port = self.config.getint('MQTT Connection', 'port', fallback=8883)
        self.tls_params = tls_params
        self.protocol = aiomqtt.ProtocolVersion.V5
        #self.protocol = aiomqtt.ProtocolVersion.V311

        self.output_dir = Path(self.config.get('Output', 'output_dir', fallback='.'))


    def __repr__(self) -> str:
        tmp = {
            'hostname': self.hostname,
            'port': self.port,
            'tls_params': self.tls_params,
            'protocol': self.protocol,
            'output_dir': pformat(self.output_dir),
        }
        return pformat(tmp)


    def get_cert_dir(self) -> os.PathLike:
        """
        """
        # Check the config file for a certificate directory.
        cert_dir = self.config.get('MQTT Connection', 'cert_dir', fallback='')
        if cert_dir and os.path.isdir(cert_dir):
            return cert_dir

        # And then try directories named 'private'.
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


    def get_output_filename(self) -> os.PathLike:
        """
        Return a filename based on the current date.
        """
        now = datetime.now()
        filename = now.strftime("soilmoisture_%Y-%m-%d.csv")
        #filename = 'soilmoisture_2023-11-10.csv'
        return self.output_dir / filename


    async def listen_for_moisture_values(self, mqtt_listen_queue: QueueType):
        """
        """
        async with aiomqtt.Client(
            hostname=self.hostname,
            port=self.port,
            tls_params=self.tls_params,
            protocol=self.protocol
        ) as client:
            async with client.messages() as messages:
                await client.subscribe("/soilmoisture/#")
                async for message in messages:
                    msg_str = '{}, {}'.format(
                        message.topic.value,
                        message.payload.decode('utf-8')
                    )
                    await mqtt_listen_queue.put(msg_str)
                    logger.info('message: ' + msg_str)


    async def save_values_to_file(self, mqtt_listen_queue: QueueType):
        """
        """
        value_to_save = None

        while True:
            # asynchronously wait for the next queued value.
            logger.debug('waiting for queue...')
            value_to_save = await mqtt_listen_queue.get()
            logger.debug('got: ' + value_to_save)

            try:
                # Get the output filename every time because the name is based on the current date.
                # When the date changes the filename changes.
                output_filename = self.get_output_filename()
                logger.debug(f'opening file: {output_filename}')
                async with aiofiles.open(output_filename, mode='a', encoding='utf-8') as out_file:
                    while True:
                        # Save the last value gotten.
                        await out_file.write(value_to_save + "\n")
                        mqtt_listen_queue.task_done()
                        logger.debug('wrote: ' + value_to_save)

                        #------------------------------------------------------------------------------------
                        # Wait for the next queued item, or two seconds, which ever comes first.
                        # If we got a queued item then continue the inside loop,
                        #  then, at the top of the inside loop, save the value just gotten.
                        # If we timed-out then catch the TimeoutError exception out side of the inside loop,
                        #  and also out side of the 'with open(...)'.
                        #  This is carefully orchestrated to save and close the file,
                        #  and then start waiting for the next queued item without the file open because
                        #  it has been a while since the last value was posted and received.
                        #  We don't want to keep the file open if it is not needed.
                        #------------------------------------------------------------------------------------
                        logger.debug('waiting for queue or 2 seconds...')
                        value_to_save = await asyncio.wait_for(mqtt_listen_queue.get(), timeout=2.0)
                        logger.debug('got: ' + value_to_save)

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
        mqtt_listen_queue: QueueType = asyncio.Queue()

        async with asyncio.TaskGroup() as task_group:
            task_group.create_task(self.listen_for_moisture_values(mqtt_listen_queue))
            task_group.create_task(self.save_values_to_file(mqtt_listen_queue))



#-------------------------------------------------------------------------------
def commandLineArgs():
    parser = argparse.ArgumentParser()
    parser.add_argument("--config", default=DEFAULT_CONFIG_FILENAME,  help="Configuration filename.")
    parser.add_argument("--debug", action="store_true", help="Run in debug mode.")
    return parser.parse_args()


def main(args) -> None:
    """
    """
    save_mqtt_messages = SaveMqttMessages(args)

    if args.debug:
        asyncio.run(save_mqtt_messages.start(), debug=True)
    else:
        asyncio.run(save_mqtt_messages.start())



#-------------------------------------------------------------------------------
logger.debug('name: {}'.format(__name__))
if __name__ == "__main__":
    args = commandLineArgs()
    if args.debug:
        logging.basicConfig(level=logging.DEBUG)
    else:
        logging.basicConfig(level=logging.INFO)
    main(args)
