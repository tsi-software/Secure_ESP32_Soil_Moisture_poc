# create_client_uuid.py
import argparse
import logging
import os
from pathlib import Path
from uuid import uuid4

logger = logging.getLogger('create_client_uuid')

SCRIPT_NAME = Path(os.path.realpath(__file__))
SCRIPT_PATH  = Path(os.path.dirname(os.path.realpath(__file__)))
DEFAULT_TARGET_PATH = (SCRIPT_PATH / '..' / 'private').resolve()
DEFAULT_FILENAME = DEFAULT_TARGET_PATH  / 'client_id.uuid4'

def create_client_uuid(args):
    """
    """
    uuid_filename = DEFAULT_FILENAME
    if args.filename:
        uuid_filename = Path(args.filename)

    if uuid_filename.exists() and not args.force:
        logger.warning('File Exists: {}'.format(str(uuid_filename)))
        return

    id = uuid4()
    id_str = str(id)
    #id_str = id.hex

    if args.force:
        logger.warning('Overwriting "{}"'.format(str(uuid_filename)))

    print('Saving UUID: {}'.format(id_str))
    print('         to: {}'.format(str(uuid_filename)))
    with open(uuid_filename, mode='wt') as f:
        f.write(id_str)


def parse_args():
    # see https://docs.python.org/3/library/argparse.html
    parser = argparse.ArgumentParser(description='Create Client UUID.')

    parser.add_argument('--filename',
                        help='The filename is which to save the Client UUID. default="{}"'.format(str(DEFAULT_FILENAME)))
    parser.add_argument('--force', action="store_true",
                        help='Force the file to be regenerated and overwrite the file if it exists.')

    args = parser.parse_args()
    return args


def main():
    """
    """
    logger.debug('SCRIPT_NAME="%s"', SCRIPT_NAME)
    logger.debug('SCRIPT_PATH="%s"', SCRIPT_PATH)
    logger.debug('DEFAULT_TARGET_PATH="%s"', DEFAULT_TARGET_PATH)
    logger.debug('DEFAULT_FILENAME="%s"', DEFAULT_FILENAME)

    args = parse_args()
    create_client_uuid(args)
    logger.debug('Done.')


if __name__ == "__main__":
    # see https://docs.python.org/3/howto/logging.html
    logging.basicConfig(
        format='%(levelname)s:%(message)s',
        encoding='utf-8',
        level=logging.INFO
    )
    main()
