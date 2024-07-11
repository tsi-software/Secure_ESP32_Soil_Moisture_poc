# -*- coding: utf-8 -*-
# tests/test_soilmoisture_graph.py
#
# To run tests:
# python3 -m unittest test_soilmoisture_graph.py
#
import argparse
import configparser
from datetime import datetime, timedelta, timezone, tzinfo
import logging
from pathlib import Path
import sys
import unittest
from unittest.mock import Mock, MagicMock

sys.path.append('..')
import soilmoisture_graph


config_test_str = """
"""



class TestSoilmoistureGraph(unittest.TestCase):
    def setUp(self):
        self.args = Mock()
        self.args.debug = True

        # see: https://docs.python.org/3/library/configparser.html
        self.config = configparser.ConfigParser()
        self.config.read_string(config_test_str)



    def test_date_range_generator(self):
        self.args.days = 3
        from_timestamp_utc = datetime(2024, 7, 9, tzinfo=timezone.utc).timestamp()
        to_timestamp_utc = datetime(2024, 7, 11, tzinfo=timezone.utc).timestamp()

        expected = [
            datetime(2024, 7, 9, 0, 0, tzinfo=timezone.utc),
            datetime(2024, 7, 10, 0, 0, tzinfo=timezone.utc),
            datetime(2024, 7, 11, 0, 0, tzinfo=timezone.utc),
        ]
        actual = [date for date in soilmoisture_graph.date_range_generator(from_timestamp_utc, to_timestamp_utc)]
        self.assertEqual(expected, actual)

        #-----------------------------------------------------------------------
        self.args.hours = 12
        from_timestamp_utc = 1720637387.631707
        to_timestamp_utc = 1720680587.631707

        expected = [
            datetime(2024, 7, 10, 18, 49, 47, 631707, tzinfo=timezone.utc),
            datetime(2024, 7, 11,  6, 49, 47, 631707, tzinfo=timezone.utc),
        ]
        actual = [date for date in soilmoisture_graph.date_range_generator(from_timestamp_utc, to_timestamp_utc)]
        self.assertEqual(expected, actual)



    def test_get_data_filenames(self):
        self.args.days = 3
        from_timestamp_utc = datetime(2024, 7, 9, tzinfo=timezone.utc).timestamp()
        to_timestamp_utc = datetime(2024, 7, 11, tzinfo=timezone.utc).timestamp()

        expected = [
            Path('output_data/soilmoisture_2024-07-09_UTC.csv'),
            Path('output_data/soilmoisture_2024-07-10_UTC.csv'),
            Path('output_data/soilmoisture_2024-07-11_UTC.csv'),
        ]
        actual = soilmoisture_graph.get_data_filenames(self.args, self.config, from_timestamp_utc, to_timestamp_utc)
        self.assertEqual(expected, actual)



if __name__ == '__main__':
    logging.basicConfig(level=logging.DEBUG)
    unittest.main()
