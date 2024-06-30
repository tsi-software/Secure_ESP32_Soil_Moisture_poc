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
from datetime import datetime, timedelta, timezone, tzinfo
import json
import logging
import os
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from pathlib import Path
from pprint import pformat
#from typing import TypeAlias

logger = logging.getLogger('soilmoisture_graph')

DEFAULT_CONFIG_FILENAME = 'config.ini'

sensor_meta_data = {}



#-------------------------------------------------------------------------------
def get_data_dir(args, config):
    """
    """
    data_dirs = [
        'output_data',
        '../output_data',
        '../../output_data',
    ]

    config_output_dir = config.get('Output', 'output_dir', fallback=None)
    if config_output_dir is not None:
        data_dirs.insert(0, config_output_dir)
        logger.debug(f'{config_output_dir=}')
    else:
        logger.debug("config['Output']['output_dir'] NOT GIVEN.")

    for test_dir in data_dirs:
        if os.path.isdir(test_dir):
            logger.info(f'data dir: {test_dir}')
            return test_dir

    raise Exception('Sensor Data directory NOT FOUND!')



def get_data_filenames(args, config):
    """
    """
    now = datetime.now(timezone.utc)

    sensor_data_dir = Path(get_data_dir(args, config))
    filename_format = 'soilmoisture_{}*.csv'

    logger.debug('now: ' + now.strftime('%Y-%m-%d'))
    logger.debug(f'{args.days=}')

    # Add 1 to the 'number of days' in case that one day earlier happens to contain "spill-over" data.
    # This also handles UTC data converted to another timezone.
    sample_dates = [now-timedelta(days=day) for day in range(args.days)]
    logger.debug('sample_dates:\n{}'.format(pformat(sample_dates)))

    # e.g.
    # sensor_data_filenames = [
    #     sensor_data_dir / 'soilmoisture_2023-11-14.csv',
    #     sensor_data_dir / 'soilmoisture_2023-11-15.csv',
    #     sensor_data_dir / 'soilmoisture_2024-06-19_PDT.csv',
    # ]
    sensor_data_filenames = []
    for sample_date in sample_dates:
        # e.g. sensor_data_dir / 'soilmoisture_2023-11-14_PDT.csv'
        sample_files = sensor_data_dir.glob( filename_format.format(sample_date.strftime('%Y-%m-%d')) )
        for sample_file in sample_files:
            sensor_data_filenames.append(sample_file)
    #
    logger.info('sensor_data_filenames:\n{}'.format(pformat(sensor_data_filenames)))

    return sensor_data_filenames


def find_sensor_meta_data(sensor_uuid, sensor_meta_data):
    """
    """
    for sensor in sensor_meta_data['sensors']:
        if sensor['uuid'] == sensor_uuid:
            return sensor

    logger.warning('find_sensor_meta_data(...): "{}" NOT FOUND!'.format(sensor_uuid))
    return None


def from_sensor_id(sensor_id, fieldname, sensor_meta_data, sensor_info_cache):
    """
    """
    if sensor_id not in sensor_info_cache:
        # Populate the 'sensor_info_cache' for 'sensor_id'
        #   example sensor_id:
        #   soilmoisture/517b462f-34ba-4c10-a41a-2310a8acd626/touchpad/1
        parts = sensor_id.split('/')
        sensor_uuid = parts[1]
        sensor_port = int(parts[3])

        metadata = find_sensor_meta_data(sensor_uuid, sensor_meta_data)
        if metadata:
            sensor_name = metadata['name']
        else:
            sensor_name = sensor_uuid

        sensor_label = f'{sensor_name} ({sensor_port})'

        sensor_info_cache[sensor_id] = {
            'sensor_uuid': sensor_uuid,
            'sensor_name': sensor_name,
            'sensor_port': sensor_port,
            'sensor_label': sensor_label,
        }

    return sensor_info_cache[sensor_id][fieldname]


def from_utc_timestamp(utc_timestamp):
    """
    """
    return datetime.fromtimestamp(int(utc_timestamp), tz=timezone.utc)


def read_sensor_data(args, config, sensor_meta_data):
    """
    """
    # sensor_info_cache is keyed on the full sensor_id.
    sensor_info_cache = {}
    dataframes = []
    for filename in get_data_filenames(args, config):
        sensor_data = pd.read_csv(filename)
        sensor_data['utc_date'] = sensor_data['utc_timestamp'].apply(from_utc_timestamp)
        sensor_data['sensor_uuid'] = sensor_data['sensor_id'].apply(from_sensor_id, args=('sensor_uuid', sensor_meta_data, sensor_info_cache))
        sensor_data['sensor_name'] = sensor_data['sensor_id'].apply(from_sensor_id, args=('sensor_name', sensor_meta_data, sensor_info_cache))
        sensor_data['sensor_port'] = sensor_data['sensor_id'].apply(from_sensor_id, args=('sensor_port', sensor_meta_data, sensor_info_cache))
        sensor_data['sensor_label'] = sensor_data['sensor_id'].apply(from_sensor_id, args=('sensor_label', sensor_meta_data, sensor_info_cache))
        #sensor_data = sensor_data.drop(columns='utc_timestamp')

        #print(sensor_data.columns)
        #print(sensor_data.head())

        dataframes.append(sensor_data)

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

    return pd.concat(dataframes, ignore_index=True)



def plot_sensor_data(args, config, sensor_data):
    """
    """
    plot_data = sensor_data
    #plot_data = sensor_data[(sensor_data.sensor_name == '#3') & (sensor_data.sensor_port == 4)]

        #[sensor_data.sensor_name == '#2'] \
        #[sensor_data.sensor_id.str.startswith('soilmoisture/517b462f-34ba-4c10-a41a-2310a8acd626/')] \
        #[sensor_data.sensor_id.str.startswith('soilmoisture/3f36213a-ec4b-43ea-8a85-ac6098fac883/')] \
        #[sensor_data['sensor_id'].str.startswith('soilmoisture/3f36213a-ec4b-43ea-8a85-ac6098fac883/')] \
        #[sensor_data['sensor_id'] != 'soilmoisture/1/capacitive/9'] \

    plot_data = plot_data \
        .sort_values(by='utc_date') \
        .pivot(columns='sensor_label', index='utc_date', values='sensor_value')

    # print(plot_data.index)
    # print(plot_data.columns)
    # print(plot_data.head())
    # print(plot_data.tail())

    # see:
    # https://pandas.pydata.org/docs/user_guide/visualization.html
    # https://pandas.pydata.org/docs/user_guide/cookbook.html#plotting
    # https://pandas.pydata.org/docs/reference/api/pandas.DataFrame.plot.html
    matplotlib_axes = plot_data.plot(
        kind = 'line',
        subplots = [
            ('#2 (1)', '#2 (2)', '#2 (3)', '#2 (4)'),
            ('#3 (1)', '#3 (2)', '#3 (3)', '#3 (4)')
        ],
        sharex = True,
        sharey = False,
        color  = {
            '#2 (1)':'blue', '#2 (2)':'orange', '#2 (3)':'green', '#2 (4)':'red',
            '#3 (1)':'black', '#3 (2)':'cyan', '#3 (3)':'green', '#3 (4)':'red',
        },
    )
    #color  = ('Red', 'Green', 'Blue', 'Orange', 'black', 'pink', 'cyan', 'yellow'),
    #layout = (1,2), #(rows,cols)

    # https://pandas.pydata.org/docs/user_guide/visualization.html#automatic-date-tick-adjustment
    # Iterate each subplot.
    for ax in matplotlib_axes:
        ax.figure.autofmt_xdate()

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

    with open('sensors.json') as json_fp:
        sensor_meta_data = json.load(json_fp)

    sensor_data = read_sensor_data(args, config, sensor_meta_data)
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
