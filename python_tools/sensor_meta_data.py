# -*- coding: utf-8 -*-
# sensor_meta_data.py
#

import json
import logging
from pprint import pformat

logger = logging.getLogger('sensor_meta_data')



class SensorMetaData:
    """
    """

    def __init__(self, json_filename):
        """
        """
        # sensor_info_cache is keyed on the full sensor_id, not just the uuid.
        self.sensor_info_cache = {}

        with open(json_filename) as json_fp:
            self.raw_json_data = json.load(json_fp)



    def format_touch_sensor_label(self, sensor_name, sensor_port):
        """
        """
        return f'{sensor_name} ({sensor_port})'



    def find_sensor_uuid(self, sensor_uuid):
        """
        """
        for sensor in self.raw_json_data['controllers']:
            if sensor['uuid'] == sensor_uuid:
                return sensor

        logger.warning('find_sensor_uuid(...): "{}" NOT FOUND!'.format(sensor_uuid))
        return None



    def from_sensor_id(self, sensor_id, fieldname):
        """
        """
        if sensor_id not in self.sensor_info_cache:
            # Populate 'self.sensor_info_cache' for the given 'sensor_id'.
            #   example sensor_id:
            #   soilmoisture/517b462f-34ba-4c10-a41a-2310a8acd626/touchpad/1
            parts = sensor_id.split('/')
            sensor_uuid = parts[1]
            sensor_port = int(parts[3])

            metadata = self.find_sensor_uuid(sensor_uuid)
            if metadata:
                sensor_name = metadata['name']
            else:
                sensor_name = sensor_uuid

            sensor_label = self.format_touch_sensor_label(sensor_name, sensor_port)
            #sensor_label = f'{sensor_name} ({sensor_port})'

            self.sensor_info_cache[sensor_id] = {
                'sensor_uuid': sensor_uuid,
                'sensor_name': sensor_name,
                'sensor_port': sensor_port,
                'sensor_label': sensor_label,
            }

        return self.sensor_info_cache[sensor_id][fieldname]



    def iterate_controllers(self):
        """
        Yield controller dictionaries.
        """
        key = 'controllers'
        for controller in self.raw_json_data[key]:
            yield controller



    def iterate_touch_sensors(self, controller):
        """
        Yield touch_sensor dictionaries with controller key,values merged in.
        """
        key = 'touch_sensors'

        # In case the controller is not configured with Touch Sensors.
        if key in controller:
            controller_copy = controller.copy()
            del controller_copy[key]

            for touch_sensor in controller[key]:
                yield controller_copy | touch_sensor



    def get_subplot_groups(self):
        """
        Example:
        return [
            ('#2 (1)', '#2 (2)', '#2 (3)', '#2 (4)'),
            ('#3 (1)', '#3 (2)', '#3 (3)', '#3 (4)'),
        ]
        """
        result = []

        for controller in self.iterate_controllers():
            group = []

            for touch_sensor in self.iterate_touch_sensors(controller):
                sensor_label = self.format_touch_sensor_label(touch_sensor['name'], touch_sensor['label'])
                group.append(sensor_label)

            if bool(group):
                result.append(group)

        logger.debug(pformat(result))
        return result



    def get_touch_sensor_colors(self):
        """
        Return a dictionary of touch_sensor labels and their colors.
        Example:
        return {
            '#2 (1)':'blue', '#2 (2)':'orange', '#2 (3)':'green', '#2 (4)':'red',
            '#3 (1)':'black', '#3 (2)':'cyan', '#3 (3)':'green', '#3 (4)':'red',
        }
        """
        default_color = 'grey'
        result = {}

        for controller in self.iterate_controllers():
            for touch_sensor in self.iterate_touch_sensors(controller):
                sensor_label = self.format_touch_sensor_label(touch_sensor['name'], touch_sensor['label'])

                if 'color' in touch_sensor:
                    color = touch_sensor['color']
                else:
                    color = default_color

                result[sensor_label] = color

        logger.debug(pformat(result))
        return result



#-------------------------------------------------------------------------------
logger.debug('name: {}'.format(__name__))
if __name__ == "__main__":
    #TODO: run unit tests.
    pass
