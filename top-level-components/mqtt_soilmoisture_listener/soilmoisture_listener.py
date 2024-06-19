# -*- coding: utf-8 -*-
# soilmoisture_listener.py
# Written for python 3.11
#
import asyncio
import aiofiles
import aiomqtt
import argparse
import configparser
from datetime import datetime, timezone
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
        Default config values are defined in this function.
        """
        self.args = args

        self.config = configparser.ConfigParser()
        self.config.read(self.args.config)

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
        self.mqtt_topic = self.config.get('MQTT Connection', 'topic', fallback='soilmoisture/#')

        self.output_dir = Path(self.config.get('Output', 'output_dir', fallback='output_data'))
        self.output_filename_prefix = self.config.get('Output', 'filename_prefix', fallback='soilmoisture_')
        if '/' in self.output_filename_prefix:
            raise Exception('Config Error: Output filename_prefix cannot contain a "/"')


    def __repr__(self) -> str:
        tmp = {
            'hostname': self.hostname,
            'port': self.port,
            'tls_params': self.tls_params,
            'protocol': self.protocol,
            'mqtt_topic': self.mqtt_topic,
            'output_dir': pformat(self.output_dir),
            'output_filename_prefix': self.output_filename_prefix,
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


    async def listen_for_moisture_values(self, mqtt_listen_queue: QueueType):
        """
        """
        logger.info(f'listen_for_moisture_values(...)\n{self}')

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
                output_filename_exists = output_filename.exists()
                logger.debug(f'opening file: {output_filename}')
                async with aiofiles.open(output_filename, mode='a', encoding='utf-8') as out_file:
                    if not output_filename_exists:
                        #-----------------------------------------------------------
                        # The first row written must be the column header.
                        # This will be needed later when the csv file is processed.
                        #-----------------------------------------------------------
                        await out_file.write(self.get_output_header() + "\n")
                        output_filename_exists = True
                    #
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
    parser = argparse.ArgumentParser(
            description='Listen for, and save, MQTT messages with the Topic "soilmoisture/#"'
        )
    parser.add_argument("--config", default=DEFAULT_CONFIG_FILENAME,  help="Configuration filename. (default: %(default)s)")
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
