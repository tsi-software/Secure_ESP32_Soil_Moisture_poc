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

from controller_meta_data import ControllerMetaData

logger = logging.getLogger('soilmoisture_graph')

DEFAULT_CONFIG_FILENAME = 'config.ini'



#-------------------------------------------------------------------------------
class DistinctControllersAndPorts:
    """
    """
    def __init__(self, dataframe):
        """
        """
        #self.distinct_controllers = {}
        self.process_dataframe(dataframe)


    def process_dataframe(self, dataframe):
        """
        """
        logger.info(f'process_dataframe(...):')
        # logger.info(dataframe.info())
        # logger.info(dataframe.head())

        self.distinct_controllers_and_ports = dataframe \
            .drop_duplicates(['controller_uuid', 'sensor_port']) \
            .sort_values(by=['controller_name', 'sensor_port']) \
            .reset_index(drop=True)

        #.drop_duplicates(['controller_uuid', 'sensor_port']) [['controller_uuid', 'controller_name', 'sensor_label', 'sensor_port']]

        logger.info(self.distinct_controllers_and_ports.info())
        logger.info(self.distinct_controllers_and_ports.head(n=32))

        self.distinct_controllers = self.distinct_controllers_and_ports \
            .drop_duplicates(['controller_uuid']) [['controller_uuid', 'controller_name']]
        logger.info(self.distinct_controllers.head(n=32))


    def get_subplot_groups(self):
        """
        Example:
        return [
            ('#2 (1)', '#2 (2)', '#2 (3)', '#2 (4)'),
            ('#3 (1)', '#3 (2)', '#3 (3)', '#3 (4)'),
        ]
        """
        result = []

        for name, group in self.distinct_controllers_and_ports.groupby('controller_uuid'):
            grouped_labels = [label for label in group['sensor_label']]
            result.append(grouped_labels)

        logger.debug(f'get_subplot_groups():\n{pformat(result)}')
        return result


    def get_touch_sensor_line_colors(self):
        """
        Return a dictionary of touch_sensor labels and their line colors.
        Example:
        return {
            '#2 (1)':'blue', '#2 (2)':'orange', '#2 (3)':'green', '#2 (4)':'red',
            '#3 (1)':'black', '#3 (2)':'cyan', '#3 (3)':'green', '#3 (4)':'red',
        }
        """
        df = self.distinct_controllers_and_ports
        result = {label:color for label,color in zip(df['sensor_label'], df['sensor_color'])}
        logger.debug(f'get_touch_sensor_line_colors():\n{pformat(result)}')
        return result



    def get_touch_sensor_line_styles(self):
        """
        Return a dictionary of touch_sensor labels and their line styles.
        Example:
        return {
            '#2 (1)':'-', '#2 (2)':':', '#2 (3)':'--', '#2 (4)':'-.',
            '#3 (1)':'-', '#3 (2)':':', '#3 (3)':'--', '#3 (4)':'-.',
        }
        """
        df = self.distinct_controllers_and_ports
        result = {label:style for label,style in zip(df['sensor_label'], df['sensor_line_style'])}
        logger.debug(f'get_touch_sensor_line_styles():\n{pformat(result)}')
        return result



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



def get_data_filenames(args, config, from_timestamp_utc, to_timestamp_utc):
    """
    ALL DATES are stored internally as UTC!
    Local time zones, if used at all, are only referenced when data is finally displayed.
    """
    sensor_data_dir = Path(get_data_dir(args, config))
    filename_format = 'soilmoisture_{}*.csv'

    sample_dates = [date for date in date_range_generator(from_timestamp_utc, to_timestamp_utc)]
    logger.info('sample_dates:\n{}'.format(pformat(sample_dates)))

    # e.g.
    # sensor_data_filenames = [
    #     sensor_data_dir / 'soilmoisture_2023-11-14.csv',
    #     sensor_data_dir / 'soilmoisture_2023-11-15.csv',
    #     sensor_data_dir / 'soilmoisture_2024-06-19_PDT.csv',
    #     sensor_data_dir / 'soilmoisture_2024-06-19_UTC.csv',
    # ]
    sensor_data_filenames = []
    for sample_date in sample_dates:
        # e.g. sensor_data_dir / 'soilmoisture_2023-11-14_UTC.csv'
        sample_files = sensor_data_dir.glob( filename_format.format(sample_date.strftime('%Y-%m-%d')) )
        for sample_file in sample_files:
            sensor_data_filenames.append(sample_file)
    #
    logger.info('sensor_data_filenames:\n{}'.format(pformat(sensor_data_filenames)))

    return sensor_data_filenames



#TODO: rename to 'to_datetime'
def from_utc_timestamp(utc_timestamp):
    """
    """
    return datetime.fromtimestamp(int(utc_timestamp), tz=timezone.utc).astimezone()



def date_range_generator(from_timestamp_utc: int, to_timestamp_utc: int):
    """
    """
    iter_date = datetime.fromtimestamp(from_timestamp_utc, tz=timezone.utc)
    iter_date.replace(hour=0, minute=0, second=0, microsecond=0)

    end_date = datetime.fromtimestamp(to_timestamp_utc, tz=timezone.utc)
    end_date.replace(hour=0, minute=0, second=0, microsecond=0)

    step = timedelta(days=1)

    # Handle the case when from_timestamp_utc and to_timestamp_utc
    # are both on the same day but at different times.
    while iter_date < end_date:
        yield iter_date
        iter_date += step

    # This helps handle the case when from_timestamp_utc and to_timestamp_utc
    # are both on the same day but at different times.
    yield datetime.fromtimestamp(to_timestamp_utc, tz=timezone.utc)



def get_data_date_range(args):
    """
    """
    now = datetime.now(timezone.utc)
    logger.info('now: ' + now.strftime('%Y-%m-%d %H:%M:%S %Z'))
    logger.info(f'{args.days=}, {args.hours=}')

    to_datetime = now

    if args.hours is None:
        # Calculate Days.
        from_datetime = now - timedelta(days=args.days)
    else:
        # Calculate Hours.
        from_datetime = now - timedelta(hours=args.hours)

    logger.info('from: ' + from_datetime.strftime('%Y-%m-%d %H:%M:%S %Z'))

    from_timestamp_utc = from_datetime.timestamp()
    to_timestamp_utc  =  to_datetime.timestamp()
    logger.info(f'{from_timestamp_utc=}, {to_timestamp_utc=}')
    return from_timestamp_utc, to_timestamp_utc



def read_sensor_data(args, config, controller_metadata):
    """
    """
    from_timestamp_utc, to_timestamp_utc = get_data_date_range(args)

    dataframes = []
    for filename in get_data_filenames(args, config, from_timestamp_utc, to_timestamp_utc):
        sensor_data = pd.read_csv(filename)

        # Filter out data outside of the desire date range.
        sensor_data = sensor_data[
          (sensor_data.utc_timestamp >= from_timestamp_utc) & (sensor_data.utc_timestamp <= to_timestamp_utc)
        ]
        if sensor_data.empty:
            continue

        sensor_data['utc_date'] = sensor_data['utc_timestamp'].apply(from_utc_timestamp)

        # Add new columns based on the values encoded in sensor_id.
        sensor_data['controller_uuid'] = sensor_data['sensor_id'].apply(
            lambda sensor_id: controller_metadata.from_sensor_id(sensor_id, 'controller_uuid')
        )
        sensor_data['controller_name'] = sensor_data['sensor_id'].apply(
            lambda sensor_id: controller_metadata.from_sensor_id(sensor_id, 'controller_name')
        )
        sensor_data['sensor_port'] = sensor_data['sensor_id'].apply(
            lambda sensor_id: controller_metadata.from_sensor_id(sensor_id, 'sensor_port')
        )
        sensor_data['sensor_label'] = sensor_data['sensor_id'].apply(
            lambda sensor_id: controller_metadata.from_sensor_id(sensor_id, 'sensor_label')
        )
        sensor_data['sensor_color'] = sensor_data['sensor_id'].apply(
            lambda sensor_id: controller_metadata.from_sensor_id(sensor_id, 'sensor_color')
        )
        sensor_data['sensor_line_style'] = sensor_data['sensor_id'].apply(
            lambda sensor_id: controller_metadata.from_sensor_id(sensor_id, 'sensor_line_style')
        )

        #sensor_data = sensor_data.drop(columns='utc_timestamp')
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

    if not dataframes:
        return None
    else:
        return pd.concat(dataframes, ignore_index=True)



def plot_sensor_data(args, config, sensor_data, controller_metadata):
    """
    """
    dt_now = datetime.now(timezone.utc).astimezone()

    #TODO: CLEAN THIS UP!
    #--------------------------------------------------------------------------------------------------
    # Manually filter some of the data.
    plot_data = sensor_data
    #plot_data = sensor_data[(sensor_data.controller_name == '#4')]
    #plot_data = sensor_data[(sensor_data.controller_name == '#3') & (sensor_data.sensor_port == 4)]

        #[sensor_data.controller_name == '#2'] \
        #[sensor_data.sensor_id.str.startswith('soilmoisture/517b462f-34ba-4c10-a41a-2310a8acd626/')] \
        #[sensor_data.sensor_id.str.startswith('soilmoisture/3f36213a-ec4b-43ea-8a85-ac6098fac883/')] \
        #[sensor_data['sensor_id'].str.startswith('soilmoisture/3f36213a-ec4b-43ea-8a85-ac6098fac883/')] \
        #[sensor_data['sensor_id'] != 'soilmoisture/1/capacitive/9'] \
    #--------------------------------------------------------------------------------------------------

    # This must be called BEFORE the call to .pivot(...) below.
    controllers_and_ports = DistinctControllersAndPorts(plot_data)

    plot_data = plot_data \
        .sort_values(by='utc_date') \
        .pivot(columns='sensor_label', index='utc_date', values='sensor_value') \
        .interpolate(method='linear')

    # logger.debug(plot_data.info())
    # logger.debug(plot_data.index)
    # logger.debug(plot_data.columns)
    # logger.debug(plot_data.head())
    # logger.debug(plot_data.tail())
    # plot_data.to_csv("tmp_plot_data.csv")

    label_x = f'Date & Hour ({dt_now.tzname()})'

    # see:
    # https://pandas.pydata.org/docs/user_guide/visualization.html
    # https://pandas.pydata.org/docs/user_guide/cookbook.html#plotting
    # https://pandas.pydata.org/docs/reference/api/pandas.DataFrame.plot.html
    matplotlib_axes = plot_data.plot(
        title = 'Soil Moisture Sensor',
        xlabel = label_x,
        kind = 'line',
        subplots = controllers_and_ports.get_subplot_groups(),
        color = controllers_and_ports.get_touch_sensor_line_colors(),
        style = controllers_and_ports.get_touch_sensor_line_styles(),
        sharex = True,
        sharey = False,
    )
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
    mutex_group = parser.add_mutually_exclusive_group()
    mutex_group.add_argument("--days", type=int, default=3,  help="Display sensor data for the given number of days. (default: %(default)s)")
    mutex_group.add_argument("--hours", type=int,  help="Display sensor data for the given number of hours.")
    return parser.parse_args()


def main(args) -> None:
    """
    """
    config = configparser.ConfigParser()
    config.read(args.config)

    controller_metadata = ControllerMetaData('controller-meta-data.json')

    sensor_data = read_sensor_data(args, config, controller_metadata)
    print('')
    plot_sensor_data(args, config, sensor_data, controller_metadata)
    print('')



#-------------------------------------------------------------------------------
logger.debug('name: {}'.format(__name__))
if __name__ == "__main__":
    args = commandLineArgs()
    if args.debug:
        logging.basicConfig(level=logging.DEBUG)
    else:
        logging.basicConfig(level=logging.INFO)
    main(args)
