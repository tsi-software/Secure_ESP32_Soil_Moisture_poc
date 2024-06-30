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
        for sensor in self.raw_json_data['sensors']:
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



    def iterate_sensors(self):
        """
        """
        for sensor in self.raw_json_data['sensors']:
            yield sensor



    def iterate_touch_sensors(self):
        """
        """
        key = 'touch_sensors'

        for sensor in self.iterate_sensors():
            if key not in sensor:
                # This chip is not configured with Touch Sensors.
                continue

            sensor_copy = sensor.copy()
            del sensor_copy[key]

            for touch_sensor in sensor[key]:
                yield sensor_copy | touch_sensor



    def get_subplot_groups(self):
        """
        Example:
        return [
            ('#2 (1)', '#2 (2)', '#2 (3)', '#2 (4)'),
            ('#3 (1)', '#3 (2)', '#3 (3)', '#3 (4)'),
        ]
        """
        group = []
        result = []
        previous_name = ''
        first_time = True

        for touch_sensor in self.iterate_touch_sensors():
            sensor_label = self.format_touch_sensor_label(touch_sensor['name'], touch_sensor['label'])
            if previous_name != touch_sensor['name'] or first_time:
                first_time = False
                previous_name = touch_sensor['name']
                group = [ sensor_label ]
                result.append(group)
            else:
                group.append(sensor_label)

        logger.info(pformat(result))

        return result



    def get_line_colors(self):
        """
        """
        #JUST TESTING!
        return {
            '#2 (1)':'blue', '#2 (2)':'orange', '#2 (3)':'green', '#2 (4)':'red',
            '#3 (1)':'black', '#3 (2)':'cyan', '#3 (3)':'green', '#3 (4)':'red',
        }



#-------------------------------------------------------------------------------
logger.debug('name: {}'.format(__name__))
if __name__ == "__main__":
    #TODO: run unit tests.
    pass
