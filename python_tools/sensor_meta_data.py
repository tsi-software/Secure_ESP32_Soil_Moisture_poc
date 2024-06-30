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

            sensor_label = f'{sensor_name} ({sensor_port})'

            self.sensor_info_cache[sensor_id] = {
                'sensor_uuid': sensor_uuid,
                'sensor_name': sensor_name,
                'sensor_port': sensor_port,
                'sensor_label': sensor_label,
            }

        return self.sensor_info_cache[sensor_id][fieldname]



#-------------------------------------------------------------------------------
logger.debug('name: {}'.format(__name__))
if __name__ == "__main__":
    #TODO: run unit tests.
    pass
