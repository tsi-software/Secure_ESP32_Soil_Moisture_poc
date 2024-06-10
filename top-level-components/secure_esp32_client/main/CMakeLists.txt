idf_component_register(
    SRCS "app_main.cpp" "app_config.cpp" "app_event_loop.c" "app_mqtt50_init.c" "app_mqtt50.cpp" "app_sntp_sync_time.c" "app_timer.c" "app_touch_pads.cpp" "app_wifi_station.c" "KalmanFilter_1D.cpp"
    INCLUDE_DIRS "."
)

# Create a NVS image from the contents of the `nvs_data` CSV file
# that fits the partition named 'nvs'. FLASH_IN_PROJECT indicates that
# the generated image should be flashed when the entire project is flashed to
# the target with 'idf.py -p PORT flash'.
# see: https://cmake.org/cmake/help/latest/manual/cmake-variables.7.html
nvs_create_partition_image(nvs /project/src/main/nonvolatile_storage.csv FLASH_IN_PROJECT)

add_compile_definitions(APP_DEBUG)
add_compile_definitions(DEBUG_TOUCH_PAD_NUMBER=1)
