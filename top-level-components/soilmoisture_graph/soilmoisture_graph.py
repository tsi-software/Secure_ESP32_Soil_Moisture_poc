# -*- coding: utf-8 -*-
# soilmoisture_graph.py
#
# Setup:
# python3 -m venv .venv
# source .venv/bin/activate
# pip3 install --upgrade pip
# pip3 install -r requirements.txt
#

import argparse
import configparser
from datetime import datetime, timezone
import logging
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from pathlib import Path
#from pprint import pformat
#from typing import TypeAlias

logger = logging.getLogger('soilmoisture_graph')

DEFAULT_CONFIG_FILENAME = 'config.ini'



#-------------------------------------------------------------------------------
def from_unix_epoch(unix_timestamp):
    return pd.Timestamp(unix_timestamp, unit='s', tz=timezone.utc)



def read_sensor_data(args, config):
    """
    """
    sensor_data_dir = Path('~/github/tsi-software/Secure_ESP32_Soil_Moisture_poc/output_data')
    #sensor_data_dir = Path('/mnt/max_github/tsi-software/Secure_ESP32_Soil_Moisture_poc/output_data')
    sensor_data_filename = sensor_data_dir / 'soilmoisture_2023-11-14.csv'

    sensor_data = pd.read_csv(sensor_data_filename)
    sensor_data['utc_date'] = sensor_data['utc_timestamp'].apply(from_unix_epoch)
    #sensor_data = sensor_data.drop(columns='utc_date')

    # Column Names: sensor_id, sensor_value, utc_timestamp, utc_date
    # logger.debug(sensor_data.info())
    # logger.debug(sensor_data)
    # logger.debug(sensor_data.to_markdown())
    # 
    # pd.set_option('display.max_rows', None)
    # pd.set_option('display.max_rows', 60)
    # pd.get_option('display.max_rows')
    # pd.set_option('display.max_columns', None)  # or 1000
    # pd.set_option('display.max_rows', None)  # or 1000
    # pd.set_option('display.max_colwidth', None)  # or 199
    #
    # For full list of useful options, see:
    # pd.describe_option('display')

    return sensor_data



def plot_sensor_data(args, config, sensor_data):
    """
    """
    plot_data = sensor_data \
        [sensor_data['sensor_id'] != '/soilmoisture/1/capacitive/9'] \
        .sort_values(by='utc_date') \
        .pivot(columns='sensor_id', index='utc_date', values='sensor_value')
    plot_data.plot(kind='line')
    plt.show()



#-------------------------------------------------------------------------------
def commandLineArgs():
    parser = argparse.ArgumentParser(
            description='Graph soil moisture sensor data'
        )
    parser.add_argument("--config", default=DEFAULT_CONFIG_FILENAME,  help="Configuration filename. (default: %(default)s)")
    parser.add_argument("--debug", action="store_true", help="Run in debug mode.")
    parser.add_argument("--days", type=int, default=7,  help="Display sensor data for the given number of days. (default: %(default)s)")
    return parser.parse_args()


def main(args) -> None:
    """
    """
    config = configparser.ConfigParser()
    config.read(args.config)

    sensor_data = read_sensor_data(args, config)
    plot_sensor_data(args, config, sensor_data)



#-------------------------------------------------------------------------------
logger.debug('name: {}'.format(__name__))
if __name__ == "__main__":
    args = commandLineArgs()
    if args.debug:
        logging.basicConfig(level=logging.DEBUG)
    else:
        logging.basicConfig(level=logging.INFO)
    main(args)
