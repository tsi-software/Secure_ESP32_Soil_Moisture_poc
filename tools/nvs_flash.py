"""
nvs_flash.py

see:
https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/partition-tables.html#partition-tool-parttool-py


idf.py partition-table
...
# ESP-IDF Partition Table
# Name, Type, SubType, Offset, Size, Flags
nvs,data,nvs,0x9000,24K,
phy_init,data,phy,0xf000,4K,
factory,app,factory,0x10000,1M,
...

$IDF_PATH/components/nvs_flash/nvs_partition_generator/nvs_partition_gen.py \
 generate /project/private/nonvolatile_storage.csv /project/private/nvs_partition.bin 0x3000

$IDF_PATH/components/partition_table/parttool.py --esptool-args=trace \
  write_partition --partition-name=nvs --input=/project/private/nvs_partition.bin

esptool.py --chip esp32 -p /dev/ttyACM0 -b 460800 --before=default_reset --after=no_reset write_flash \
  --flash_mode dio --flash_freq 40m --flash_size 2MB \
  0x1000 bootloader/bootloader.bin \
  0x10000 secure-soil-moisture.bin \
  0x8000 partition_table/partition-table.bin

"""

# Before anything else, make sure that the parttool module is imported.
import sys
import os

idf_path = os.environ["IDF_PATH"]  # get value of IDF_PATH from environment
parttool_dir = os.path.join(idf_path, "components", "partition_table")  # parttool.py lives in $IDF_PATH/components/partition_table


# JUST TESTING
parttool_dir = os.path.join(".", "just_testing")


sys.path.append(parttool_dir)  # this enables Python to find parttool module
from parttool import *  # import all names inside parttool module


# Create a parttool.py target device connected to serial port.
target = ParttoolTarget("/dev/ttyACM0")


partition_name = 'nvs'
partition_file = '/project/private/nvs_partition.bin'
print(f'\n{partition_name=}, {partition_file=}')

# Erase named partition
print('Erase named partition.')
target.erase_partition(PartitionName(partition_name))

# Write the named file to the named partition
print('Write to named partition.')
target.write_partition(PartitionName(partition_name), partition_file)

# Print the size of default boot partition
print('Get partition info.')
storage = target.get_partition_info(PartitionName(partition_name))
print(storage.size)
