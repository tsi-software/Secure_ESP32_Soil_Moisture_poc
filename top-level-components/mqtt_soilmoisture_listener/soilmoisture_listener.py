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

# The Docker --volume argument is used to position the 'python_tool' sub-directory.
#sys.path.append('../python_tools')
import python_tools.soilmoisture_config_utils as secure_connection_config

logger = logging.getLogger('soilmoisture_listener')

QueueType: TypeAlias = type(asyncio.Queue)

ACTIVE_CERTIFICATES_FILENAME = 'active_certificates.vars'
DEFAULT_APP_CONFIG_FILENAME = 'soilmoisture_listener.ini'



#-------------------------------------------------------------------------------
class SaveMqttMessages:
    """
    """

    def __init__(self, args):
        """
        args is the Command Line Arguments.
        Default config values are defined in this function.
        """
        #self.cancelled = False  # EXPERIMENTAL!
        self.args = args

        #--------------------------
        # secure_connection_config
        #--------------------------
        self.secure_conn_vars = secure_connection_config.read_config_file(self.args)
        self.active_certificates = secure_connection_config.get_active_certificates_vars(self.secure_conn_vars)
        self.certs_dir = secure_connection_config.get_active_certificates_dir(self.active_certificates)
        self.connection_parameters = secure_connection_config.get_connection_parameters(
            self.secure_conn_vars,
            self.certs_dir,
            mqtt_protocol=aiomqtt.ProtocolVersion.V5
        )

        tls_params = aiomqtt.TLSParameters(
            ca_certs = self.connection_parameters['tls_params']['ca_certs'],
            certfile = self.connection_parameters['tls_params']['certfile'],
            keyfile = self.connection_parameters['tls_params']['keyfile'],
            cert_reqs = self.connection_parameters['tls_params']['cert_reqs'],
            tls_version = self.connection_parameters['tls_params']['tls_version'],
        )

        self.hostname = self.connection_parameters['hostname']
        self.mqtt_port = self.connection_parameters['port']
        self.tls_params = tls_params
        self.protocol = self.connection_parameters['protocol']

        #-------------
        # App Config:
        #-------------
        self.app_config = configparser.ConfigParser()
        self.app_config.read(self.args.config)

        self.mqtt_topic = self.app_config.get('MQTT Connection', 'topic', fallback='soilmoisture/#')
        self.output_dir = Path(self.app_config.get('Output', 'output_dir', fallback='output_data'))
        self.output_filename_prefix = self.app_config.get('Output', 'filename_prefix', fallback='soilmoisture_')
        if '/' in self.output_filename_prefix:
            raise Exception('Config Error: Output filename_prefix cannot contain a "/"')


    def __repr__(self) -> str:
        tmp = {
            'hostname': self.hostname,
            'mqtt_port': self.mqtt_port,
            'tls_params': self.tls_params,
            'protocol': self.protocol,
            'mqtt_topic': self.mqtt_topic,
            'output_dir': pformat(self.output_dir),
            'output_filename_prefix': self.output_filename_prefix,
        }
        return pformat(tmp)


    def get_output_filename(self) -> os.PathLike:
        """
        Return a filename based on the current date.
        """
        # Always use UTC
        now = datetime.now(timezone.utc)
        #now = datetime.now(timezone.utc).astimezone()
        filename = now.strftime(self.output_filename_prefix + "%Y-%m-%d_%Z.csv")
        #e.g. filename = 'soilmoisture_2023-11-10_UTC.csv'
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

        # see:
        # https://sbtinstruments.github.io/aiomqtt/subscribing-to-a-topic.html
        async with aiomqtt.Client(
            hostname=self.hostname,
            port=self.mqtt_port,
            tls_params=self.tls_params,
            protocol=self.protocol
        ) as client:
            await client.subscribe(self.mqtt_topic)
            async for message in client.messages:
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
    parser.add_argument("--active", default=ACTIVE_CERTIFICATES_FILENAME,  help="Active Certificates Filename. (default: %(default)s)")
    parser.add_argument("--config", default=DEFAULT_APP_CONFIG_FILENAME,  help="App Configuration filename. (default: %(default)s)")
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
