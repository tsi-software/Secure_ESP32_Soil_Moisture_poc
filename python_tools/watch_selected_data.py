# -*- coding: utf-8 -*-
# watch_selected_data.py
#
# Setup:
# python3 -m venv .venv
# source .venv/bin/activate
# pip3 install --upgrade pip
# pip3 install -r requirements.txt
#
import argparse
import configparser
from datetime import timedelta, datetime, tzinfo, timezone
from itertools import chain
import logging
import os
import paho.mqtt.client as mqtt
from pathlib import Path
from pprint import pformat
import ssl

logger = logging.getLogger('watch_selected_data')

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



def read_config_file(args):
    """
    """
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
            vars_file = test_dir / args.active
            logger.debug(f'vars_file: {vars_file}')
            if vars_file.is_file():
                return read_vars_file(vars_file)

    raise Exception('Config File NOT FOUND!')



def get_active_certificates_vars(config_vars) -> str:
    """
    """
    #config_vars = read_config_file(args)

    active_cert_vars = config_vars.get('ACTIVE_CERTIFICATES_DIR', fallback='')
    logger.debug(f'active_cert_vars: {active_cert_vars}')
    if active_cert_vars:
        return active_cert_vars

    raise Exception('Active Certificates NOT FOUND!')



def get_active_certificates_dir(active_certificates) -> os.PathLike:
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



def get_connection_parameters(config_vars, certs_dir):
    tls_params = {
        'ca_certs': certs_dir / 'mosq_ca.crt',
        'certfile': certs_dir / 'client_a' / 'mosq_client.crt',
        'keyfile':  certs_dir / 'client_a' / 'mosq_client.key',
        'cert_reqs': ssl.CERT_REQUIRED,
        'tls_version': ssl.PROTOCOL_TLSv1_2,
    }

    return {
        'hostname': config_vars.get('HOSTNAME'),
        'port': config_vars.getint('PORT', fallback=8883),
        'tls_params': tls_params,
        'protocol': mqtt.MQTTv5,
        'mqtt_topic': config_vars.get('TOPIC', fallback='/soilmoisture/#'),
    }
    # ... or ...
    # mqtt.MQTTv311
    # mqtt.MQTTv31



#-------------------------------------------------------------------------------
class WatchMqttMessages(mqtt.Client):
    """
    """

    def __init__(self, args):
        """
        args is the Command Line Arguments.
        """
        self.args = args
        self.config_vars = read_config_file(self.args)
        self.active_certificates = get_active_certificates_vars(self.config_vars)
        self.certs_dir = get_active_certificates_dir(self.active_certificates)
        self.connection_parameters = get_connection_parameters(self.config_vars, self.certs_dir)

        super().__init__(
            callback_api_version = mqtt.CallbackAPIVersion.VERSION2,
            userdata = self,
            protocol = self.connection_parameters['protocol'],
        )

        self.tls_set(
            ca_certs = self.connection_parameters['tls_params']['ca_certs'],
            certfile = self.connection_parameters['tls_params']['certfile'],
            keyfile = self.connection_parameters['tls_params']['keyfile'],
            cert_reqs = self.connection_parameters['tls_params']['cert_reqs'],
            tls_version = self.connection_parameters['tls_params']['tls_version'],
        )

        # self.mqtt_client.on_connect = on_connect
        # self.mqtt_client.on_message = on_message


    def connect(self):
        """
        """
        logger.debug(f'connect()\n{self}')

        super().connect(
            host = self.connection_parameters['hostname'],
            port = self.connection_parameters['port'],
            keepalive = 60
        )


    def __repr__(self) -> str:
        return pformat(self.connection_parameters)
        # tmp = {
        #     'hostname': self.hostname,
        #     'port': self.port,
        #     'tls_params': self.tls_params,
        #     'protocol': self.protocol,
        #     'mqtt_topic': self.mqtt_topic,
        # }
        # return pformat(tmp)


    # # The callback for when the client receives a CONNACK response from the server.
    # def on_connect(client, userdata, flags, reason_code, properties):
    #     self = userdata
    #     print(f"Connected with result code {reason_code}")
    #     # Subscribing in on_connect() means that if we lose the connection and
    #     # reconnect then subscriptions will be renewed.
    #     #client.subscribe("$SYS/#")


    # # The callback for when a PUBLISH message is received from the server.
    # def on_message(client, userdata, msg):
    #     self = userdata
    #     print(msg.topic + " " + str(msg.payload))


    def on_connect(self, mqttc, obj, flags, reason_code, properties):
        logger.debug("rc: "+str(reason_code))

    def on_connect_fail(self, mqttc, obj):
        logger.error("Connect failed")

    def on_message(self, mqttc, obj, msg):
        logger.debug(msg.topic+" "+str(msg.qos)+" "+msg.payload.decode('ascii'))

        topic = msg.topic.split('/')
        if 'soilmoisture' == topic[0]:
            self.soilmoisture(topic, msg)
        elif 'irrigation' == topic[0]:
            self.irrigation(topic, msg)

    def on_publish(self, mqttc, obj, mid, reason_codes, properties):
        logger.debug("mid: "+str(mid))

    def on_subscribe(self, mqttc, obj, mid, reason_code_list, properties):
        logger.debug("Subscribed: "+str(mid)+" "+str(reason_code_list))

    def on_log(self, mqttc, obj, level, string):
        logger.debug(string)



    def soilmoisture(self, topic, msg):
        """
        """
        device_lookup = {
            '05446845-5f69-4323-9061-ac2d5069992c': {
                'name': '#1',
                'device': 'ESP32-S3',
            },
            '3f36213a-ec4b-43ea-8a85-ac6098fac883': {
                'name': '#2',
                'device': 'ESP32-S3',
            },
            '517b462f-34ba-4c10-a41a-2310a8acd626': {
                'name': '#3',
                'device': 'ESP32-S2',
            },
            '3fcfc9da-f6b7-4815-841f-d822e1cf7180': {
                'name': '#4',
                'device': 'ESP32-S2',
            },
        }
        id = topic[1]
        device = device_lookup[id]
        device_name = device['name']
        port_type = topic[2]
        port_number = int(topic[3])
        touch_value, touch_timestamp = msg.payload.decode('ascii').split(',')
        touch_time = datetime.fromtimestamp(int(touch_timestamp), tz=timezone.utc).astimezone()
        touch_time_str = touch_time.strftime('%Y-%m-%d %H:%M:%S')
        message_str = f'Touch Pad: device={device_name}, port={port_number} value={touch_value}, {touch_time_str}'

        logger.debug(message_str)

        # if device_name == '#2':
        #     if port_number >= 1 and port_number <= 4:
        #         logger.info(message_str)

        # if device_name == '#3' or device_name == '#4':
        #     if port_number >= 1 and port_number <= 4:
        #         logger.info(message_str)

        if port_number >= 1 and port_number <= 2:
            logger.info(message_str)


    def irrigation(self, topic, msg):
        """
        """
        event_timestamp = datetime.now(tz=timezone.utc)
        event_time = datetime.fromtimestamp(event_timestamp, tz=timezone.utc).astimezone()
        event_time_str = event_time.strftime('%Y-%m-%d %H:%M:%S')

        message_str = f'{msg.topic} {msg.payload} {event_time_str}'
        logger.info(message_str)



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


    def listen_for_moisture_values(self, mqtt_listen_queue):
        """
        """
        logger.info(f'listen_for_moisture_values(...)\n{self}\n')

        # async with aiomqtt.Client(
        #     hostname=self.hostname,
        #     port=self.port,
        #     tls_params=self.tls_params,
        #     protocol=self.protocol
        # ) as client:
        #     async with client.messages() as messages:
        #         await client.subscribe(self.mqtt_topic)
        #         async for message in messages:
        #             msg_str = '{},{}'.format(
        #                 message.topic.value,
        #                 message.payload.decode('utf-8')
        #             )

        #             # TODO: uncomment
        #             #await mqtt_listen_queue.put(msg_str)
                    
        #             logger.info('message: ' + msg_str)


    def save_values_to_file(self, mqtt_listen_queue):
        """
        """
        value_to_save = None

        # while True:
        #     # asynchronously wait for the next queued value.
        #     logger.debug(f'waiting for queue...\n')
        #     value_to_save = await mqtt_listen_queue.get()
        #     logger.debug('got: ' + value_to_save)

        #     try:
        #         pass

        #     except TimeoutError:
        #         # Ignore TimeoutErrors here.
        #         # The intentional side effects are:
        #         #  1) Close and save the output file because no values are being processed at the moment
        #         #     and the file is not needed.
        #         #  2) Go back to the top of the outside loop where we 'async' wait for the next queued value.
        #         #  3) Because the output file has been closed it will need to be re-open,
        #         #     this gives us the opportunity to use a new filename based on the current date
        #         #     if the date has just changed.
        #         logger.debug('file flushed and closed.')
        #         pass
        #     except Exception as err:
        #         logger.error(f"Unexpected {err=}, {type(err)=}")
        #         raise



#-------------------------------------------------------------------------------
def commandLineArgs():
    parser = argparse.ArgumentParser(
            description='Listen for, and display, MQTT messages with the Topic "/soilmoisture/#"'
        )
    parser.add_argument("--active", default=ACTIVE_CERTIFICATES_FILENAME,  help="Active Certificates Filename. (default: %(default)s)")
    #parser.add_argument("--config", default=DEFAULT_CONFIG_FILENAME,  help="Configuration filename. (default: %(default)s)")
    parser.add_argument("--debug", action="store_true", help="Run in debug mode.")
    return parser.parse_args()


def main(args) -> None:
    """
    """
    watch_mqtt_messages = WatchMqttMessages(args)
    watch_mqtt_messages.connect()
    watch_mqtt_messages.subscribe(topic='#', qos=0)

    watch_mqtt_messages.loop_forever()
    # watch_mqtt_messages.loop_start()
    # ...
    # watch_mqtt_messages.loop_stop()


#-------------------------------------------------------------------------------
logger.debug('name: {}'.format(__name__))
if __name__ == "__main__":
    args = commandLineArgs()
    if args.debug:
        logging.basicConfig(level=logging.DEBUG)
    else:
        logging.basicConfig(level=logging.INFO)
    main(args)
