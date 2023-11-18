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
#import configparser
from datetime import datetime, timezone
import logging
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from pathlib import Path
from pprint import pformat
#from typing import TypeAlias

logger = logging.getLogger('soilmoisture_graph')


# def dateparse (time_in_secs):
#     return datetime.fromtimestamp(float(time_in_secs))


def from_unix_epoch(unix_timestamp):
    return pd.Timestamp(unix_timestamp, unit='s', tz=timezone.utc)


test_data_dir = Path('~/github/tsi-software/Secure_ESP32_Soil_Moisture_poc/output_data')
#test_data_dir = Path('/mnt/max_github/tsi-software/Secure_ESP32_Soil_Moisture_poc/output_data')
test_data_filename = test_data_dir / 'soilmoisture_2023-11-14.csv'

test_data = pd.read_csv(test_data_filename)
test_data['utc_date'] = test_data['utc_timestamp'].apply(from_unix_epoch)
print(test_data.info())
print(test_data)
print(test_data.to_markdown())

#test_data = test_data.drop(columns='utc_date')


#pd.Timestamp(1699943746, unit='s', tz=timezone.utc)


#FAIL!
#test_data = pd.read_csv(test_data_filename, parse_dates=['utc_timestamp'])
#test_data = pd.read_csv(test_data_filename, parse_dates=[2], infer_datetime_format=True)
#test_data = pd.read_csv(test_data_filename, parse_dates=[2], date_format='s')
#test_data['utc_date'] = pd.to_datetime(pd.Timestamp(test_data['utc_timestamp'], unit='s', tz='UTC'))
#DEPRECATION WARNING.
#test_data = pd.read_csv(test_data_filename, parse_dates=[2], date_parser=dateparse)

#pd.set_option('display.max_rows', None)
#pd.set_option('display.max_rows', 60)
#pd.get_option('display.max_rows')
#60

# pd.set_option('display.max_columns', None)  # or 1000
# pd.set_option('display.max_rows', None)  # or 1000
# pd.set_option('display.max_colwidth', None)  # or 199
# For full list of useful options, see:
# pd.describe_option('display')


#plot_data = test_data[['sensor_value','utc_date']].sort_values(by='utc_date')
plot_data = test_data \
    [test_data['sensor_id'] != '/soilmoisture/1/capacitive/9'] \
    .sort_values(by='utc_date') \
    .pivot(columns='sensor_id', index='utc_date', values='sensor_value')
plot_data.plot(kind='line')
plt.show()


#FAIL!
#test_data.plot(['sensor_value','utc_date'], kind='line')


