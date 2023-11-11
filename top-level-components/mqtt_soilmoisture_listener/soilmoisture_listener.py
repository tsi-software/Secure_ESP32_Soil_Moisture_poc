# -*- coding: utf-8 -*-
# soilmoisture_listener.py
# Written with python 3.11 features.
#
import asyncio
import aiofiles
import aiomqtt
import argparse
from datetime import datetime
import logging
import os
from pathlib import Path
from pprint import pformat
import ssl
from typing import TypeAlias

logger = logging.getLogger('soilmoisture_listener')

ConfigType: TypeAlias = dict[str, str | int]
QueueType: TypeAlias = type(asyncio.Queue)



#TODO: ???
#class SaveMqttMessages
#PersistMqttMessages
#SaveSubscribedMessages
#PersistSubscribedMessages
#ProcessMqttMessages



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



def get_config(args) -> ConfigType:
    """
    """
    config_dir: str = get_config_dir(args)
    cert_dir: str = get_cert_dir(args)

    tls_params = aiomqtt.TLSParameters(
        ca_certs = cert_dir + '/mosq_ca.crt',
        certfile = cert_dir + '/mosq_client.crt',
        keyfile  = cert_dir + '/mosq_client.key',
        cert_reqs = ssl.CERT_REQUIRED,
        #tls_version = ssl.PROTOCOL_TLS_CLIENT,
        tls_version = ssl.PROTOCOL_TLSv1_2,
    )

    #TODO: read config file from config_dir.

    return {
        'hostname': 'raspberrypi',
        'port': 8883,
        'tls_params': tls_params,
        'protocol': aiomqtt.ProtocolVersion.V5,
        #'protocol': aiomqtt.ProtocolVersion.V311,
        'output_dir': '.',
    }



def get_output_filename(config: ConfigType) -> str:
    """
    """
    now = datetime.now()
    filename = now.strftime("soilmoisture_%Y-%m-%d.csv")
    #filename = 'soilmoisture_2023-11-10.csv'
    return Path(config['output_dir']) / filename



async def listen_for_moisture_values(config: ConfigType, mqtt_listen_queue: QueueType):
    """
    """
    async with aiomqtt.Client(
        hostname=config['hostname'],
        port=config['port'],
        tls_params=config['tls_params'],
        protocol=config['protocol']
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



async def save_values_to_file(config: ConfigType, mqtt_listen_queue: QueueType):
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
            output_filename = get_output_filename(config)
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



async def start(args, config: ConfigType):
    """
    """
    logger.debug('start()')
    mqtt_listen_queue: QueueType = asyncio.Queue()

    async with asyncio.TaskGroup() as task_group:
        task_group.create_task(listen_for_moisture_values(config, mqtt_listen_queue))
        task_group.create_task(save_values_to_file(config, mqtt_listen_queue))



#-------------------------------------------------------------------------------
def commandLineArgs():
    parser = argparse.ArgumentParser()
    parser.add_argument("-c", "--certdir", help="Mosquitto security certificates directory.")
    parser.add_argument("-d", "--debug", action="store_true", help="Run in debug mode.")
    return parser.parse_args()



def main(args) -> None:
    """
    """
    config: ConfigType = get_config(args)
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
