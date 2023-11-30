/*  main.c
    Created: 2023-10-16
    Author: Warren Taylor

    This example code is in the Public Domain (or CC0 licensed, at your option.)

    Unless required by applicable law or agreed to in writing, this
    software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
    CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "app_config.hpp"
#include "app_event_loop.h"
#include "app_mqtt50.h"
#include "app_sntp_sync_time.h"
#include "app_timer.h"
#include "app_touch_pads.h"
#include "app_wifi_station.h"


static const char *LOG_TAG = "Secure_Soil_Moisture";

static esp_event_loop_handle_t app_event_loop_handle;



extern "C" void app_main(void)
{
    esp_err_t ret;

    ESP_LOGI(LOG_TAG, "[APP] Startup..");
    ESP_LOGI(LOG_TAG, "[APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
    ESP_LOGI(LOG_TAG, "[APP] IDF version: %s", esp_get_idf_version());

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("Secure_Soil_Moisture", ESP_LOG_VERBOSE);
    esp_log_level_set("app_event_loop", ESP_LOG_VERBOSE);
    esp_log_level_set("app_mqtt", ESP_LOG_VERBOSE);
    esp_log_level_set("app_sntp_sync_time", ESP_LOG_VERBOSE);
    esp_log_level_set("app_timer", ESP_LOG_VERBOSE);
    esp_log_level_set("app_touch_pads", ESP_LOG_VERBOSE);
    esp_log_level_set("app_wifi_station", ESP_LOG_VERBOSE);

    esp_log_level_set("TRANSPORT_BASE", ESP_LOG_DEBUG);
    esp_log_level_set("ESP_TLS", ESP_LOG_DEBUG);
    // esp_log_level_set("TRANSPORT_TCP", ESP_LOG_DEBUG);
    // esp_log_level_set("TRANSPORT_SSL", ESP_LOG_DEBUG);
    esp_log_level_set("TRANSPORT", ESP_LOG_DEBUG);
    esp_log_level_set("OUTBOX", ESP_LOG_DEBUG);

    // Initialize NVS
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(create_app_event_loop(&app_event_loop_handle));

    // To be more efficient with stack and memory use
    //  create separate scopes for configuration and initialization variables.
    {
        WifiConfig wifiConfig;
        const char *ssid = wifiConfig.get_ssid();
        const char *password = wifiConfig.get_password();
        app_wifi_station_init(ssid, password);
    }
    {
        app_sntp_sync_time();
        app_mqtt50_start(app_event_loop_handle);

        app_timer_init(app_event_loop_handle);
        app_read_touch_pads_init(app_event_loop_handle);
    }
}
