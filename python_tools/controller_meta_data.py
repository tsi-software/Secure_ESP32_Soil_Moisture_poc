# -*- coding: utf-8 -*-
# controller_meta_data.py
#

import json
import logging
from pprint import pformat
from typing import Union

logger = logging.getLogger('controller_meta_data')



class ControllerMetaData:
    """
    """

    def __init__(self, json_filename):
        """
        """
        # controller_info_cache is keyed on the full sensor_id, not just the uuid.
        self.controller_info_cache = {}

        with open(json_filename) as json_fp:
            self.raw_json_data = json.load(json_fp)



    def format_touch_sensor_label(self, controller_name, sensor_port):
        """
        """
        return f'{controller_name} ({sensor_port})'



    def find_controller_uuid(self, controller_uuid):
        """
        """
        for controller in self.raw_json_data['controllers']:
            if controller['uuid'] == controller_uuid:
                return controller
        #
        logger.warning('find_controller_uuid(...): "{}" NOT FOUND!'.format(controller_uuid))
        return None



    def find_touch_sensor(self, controller_metadata, sensor_port):
        """
        """
        for sensor_dict in controller_metadata.get('touch_sensors', []):
            if sensor_dict.get('port', None) == sensor_port:
                return sensor_dict
        #
        logger.warning('find_touch_sensor(...): port {} NOT FOUND!'.format(sensor_port))
        return None


    def from_sensor_id(self, sensor_id: str, fieldname: str):
        """
        """
        if sensor_id not in self.controller_info_cache:
            # Populate 'self.controller_info_cache' for the given 'sensor_id'.
            #   example sensor_id:
            #   soilmoisture/517b462f-34ba-4c10-a41a-2310a8acd626/touchpad/1
            parts = sensor_id.split('/')
            controller_uuid = parts[1]
            sensor_port = int(parts[3])

            metadata = self.find_controller_uuid(controller_uuid)
            if metadata:
                controller_name = metadata['name']
            else:
                controller_name = controller_uuid

            sensor_label = self.format_touch_sensor_label(controller_name, sensor_port)

            # defaults.
            #TODO: define these default values somewhere appropriate.
            sensor_color = 'black'
            sensor_line_style = '-'
            # JSON values if available.
            sensor = self.find_touch_sensor(metadata, sensor_port)
            if sensor is not None:
                sensor_color = sensor.get('color', sensor_color)
                sensor_line_style = sensor.get('line_style', sensor_line_style)

            self.controller_info_cache[sensor_id] = {
                'controller_uuid': controller_uuid,
                'controller_name': controller_name,
                'sensor_port': sensor_port,
                'sensor_label': sensor_label,
                'sensor_color': sensor_color,
                'sensor_line_style': sensor_line_style,
            }

        return self.controller_info_cache[sensor_id][fieldname]



    #DEPRECATED!
    def is_in_cache(self, fieldname: str, fieldvalue: Union[str,int]) -> bool:
        """
        """
        for cache_value in self.controller_info_cache.values():
            if cache_value[fieldname] == fieldvalue:
                return True

        return False



    #DEPRECATED!
    def iterate_controllers(self):
        """
        Yield controller dictionaries.
        """
        key = 'controllers'
        for controller in self.raw_json_data[key]:
            yield controller


    #DEPRECATED!
    def iterate_cached_controllers(self):
        """
        """
        for controller in self.iterate_controllers():
            if self.is_in_cache('controller_uuid', controller['uuid']):
                yield controller



    #DEPRECATED!
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


    #DEPRECATED!
    def iterate_cached_touch_sensors(self, controller):
        """
        """
        for touch_sensor in self.iterate_touch_sensors(controller):
            if self.is_in_cache('sensor_port', touch_sensor['port']):
                yield touch_sensor



    #DEPRECATED!
    def get_subplot_groups(self):
        """
        Example:
        return [
            ('#2 (1)', '#2 (2)', '#2 (3)', '#2 (4)'),
            ('#3 (1)', '#3 (2)', '#3 (3)', '#3 (4)'),
        ]
        """
        result = []

        for controller in self.iterate_cached_controllers():
            group = []

            for touch_sensor in self.iterate_cached_touch_sensors(controller):
                sensor_label = self.format_touch_sensor_label(touch_sensor['name'], touch_sensor['label'])
                group.append(sensor_label)

            if bool(group):
                result.append(group)

        logger.debug(f'get_subplot_groups():\n{pformat(result)}')
        return result



    #DEPRECATED!
    def get_touch_sensor_line_colors(self):
        """
        Return a dictionary of touch_sensor labels and their line colors.
        Example:
        return {
            '#2 (1)':'blue', '#2 (2)':'orange', '#2 (3)':'green', '#2 (4)':'red',
            '#3 (1)':'black', '#3 (2)':'cyan', '#3 (3)':'green', '#3 (4)':'red',
        }
        """
        default_color = 'grey'
        result = {}

        for controller in self.iterate_cached_controllers():
            for touch_sensor in self.iterate_cached_touch_sensors(controller):
                sensor_label = self.format_touch_sensor_label(touch_sensor['name'], touch_sensor['label'])

                #TODO: the following could be achieve with a single 'get(...)' call.
                if 'color' in touch_sensor:
                    color = touch_sensor['color']
                else:
                    color = default_color

                result[sensor_label] = color

        logger.debug(f'get_touch_sensor_line_colors():\n{pformat(result)}')
        return result



    #DEPRECATED!
    def get_touch_sensor_line_styles(self):
        """
        Return a dictionary of touch_sensor labels and their line styles.
        Example:
        return {
            '#2 (1)':'-', '#2 (2)':':', '#2 (3)':'--', '#2 (4)':'-.',
            '#3 (1)':'-', '#3 (2)':':', '#3 (3)':'--', '#3 (4)':'-.',
        }
        """
        default_style = '-'
        result = {}

        for controller in self.iterate_cached_controllers():
            for touch_sensor in self.iterate_cached_touch_sensors(controller):
                sensor_label = self.format_touch_sensor_label(touch_sensor['name'], touch_sensor['label'])

                #TODO: the following could be achieve with a single 'get(...)' call.
                if 'line_style' in touch_sensor:
                    line_style = touch_sensor['line_style']
                else:
                    line_style = default_style

                result[sensor_label] = line_style

        logger.debug(f'get_touch_sensor_line_styles():\n{pformat(result)}')
        return result



#-------------------------------------------------------------------------------
logger.debug('name: {}'.format(__name__))
if __name__ == "__main__":
    #TODO: run unit tests.
    pass
