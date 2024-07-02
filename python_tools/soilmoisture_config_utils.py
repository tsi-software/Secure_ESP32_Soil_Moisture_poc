# -*- coding: utf-8 -*-
# soilmoisture_config_utils.py
#

import configparser
from itertools import chain
import logging
import os
import paho.mqtt.client as mqtt
from pathlib import Path
from pprint import pformat
import ssl

logger = logging.getLogger('soilmoisture_config_utils')



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



def get_connection_parameters(config_vars, certs_dir, mqtt_protocol=None):
    tls_params = {
        'ca_certs': certs_dir / 'mosq_ca.crt',
        'certfile': certs_dir / 'client_a' / 'mosq_client.crt',
        'keyfile':  certs_dir / 'client_a' / 'mosq_client.key',
        'cert_reqs': ssl.CERT_REQUIRED,
        'tls_version': ssl.PROTOCOL_TLSv1_2,
    }

    if mqtt_protocol is None:
        mqtt_protocol = mqtt.MQTTv5  # from paho.mqtt.client
        # ... or ...
        # mqtt.MQTTv311
        # mqtt.MQTTv31

    return {
        'hostname': config_vars.get('HOSTNAME'),
        'port': config_vars.getint('PORT', fallback=8883),
        'tls_params': tls_params,
        'protocol': mqtt_protocol,
        'mqtt_topic': config_vars.get('TOPIC', fallback='soilmoisture/#'),
    }



#-------------------------------------------------------------------------------
logger.debug('name: {}'.format(__name__))
if __name__ == "__main__":
    #TODO: run unit tests.
    pass
