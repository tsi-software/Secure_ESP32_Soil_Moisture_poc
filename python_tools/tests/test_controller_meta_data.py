# -*- coding: utf-8 -*-
# tests/test_controller_meta_data.py
#
# To run tests:
# python3 -m unittest test_controller_meta_data.py
#
import logging
from pathlib import Path
import sys
import unittest
from unittest.mock import Mock, MagicMock

sys.path.append('..')
from controller_meta_data import ControllerMetaData



class TestControllerMetaData(unittest.TestCase):
    def test_from_sensor_id(self):
        controller_metadata = ControllerMetaData('controller-meta-data/controller-meta-data.json')

        sensor_id = 'soilmoisture/3f36213a-ec4b-43ea-8a85-ac6098fac883/touchpad/1'
        controller_uuid = controller_metadata.from_sensor_id(sensor_id, 'controller_uuid')
        self.assertEqual('3f36213a-ec4b-43ea-8a85-ac6098fac883', controller_uuid)
        controller_visible = controller_metadata.from_sensor_id(sensor_id, 'controller_visible')
        self.assertFalse(controller_visible)
        sensor_visible = controller_metadata.from_sensor_id(sensor_id, 'sensor_visible')
        self.assertTrue(sensor_visible)

        sensor_id = 'soilmoisture/3fcfc9da-f6b7-4815-841f-d822e1cf7180/touchpad/1'
        controller_visible = controller_metadata.from_sensor_id(sensor_id, 'controller_visible')
        self.assertTrue(controller_visible)
        sensor_visible = controller_metadata.from_sensor_id(sensor_id, 'sensor_visible')
        self.assertFalse(sensor_visible)



if __name__ == '__main__':
    logging.basicConfig(level=logging.DEBUG)
    unittest.main()
