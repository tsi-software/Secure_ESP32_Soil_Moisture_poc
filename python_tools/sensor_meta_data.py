# -*- coding: utf-8 -*-
# sensor_meta_data.py
#

import json
import logging
from pprint import pformat

logger = logging.getLogger('sensor_meta_data')



class sensor_meta_data:
    """
    """

    def __init__(self, json_filename):
        """
        """
        with open(json_filename) as json_fp:
            self.raw_json_data = json.load(json_fp)


#-------------------------------------------------------------------------------
logger.debug('name: {}'.format(__name__))
if __name__ == "__main__":
    #TODO: run unit tests.
    pass
