idf_component_register(
    SRCS "app_main.cpp" "app_config.cpp" "app_event_loop.c" "app_mqtt50.c" "app_sntp_sync_time.c" "app_timer.c" "app_touch_pads.c" "app_wifi_station.c"
    INCLUDE_DIRS "."
)

# Create a NVS image from the contents of the `nvs_data` CSV file
# that fits the partition named 'nvs'. FLASH_IN_PROJECT indicates that
# the generated image should be flashed when the entire project is flashed to
# the target with 'idf.py -p PORT flash'.
nvs_create_partition_image(nvs /project/private/nonvolatile_storage.csv FLASH_IN_PROJECT)
