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
//#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
//#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

//#include "lwip/err.h"
//#include "lwip/sys.h"

#include "app_event_loop.h"
#include "app_sntp_sync_time.h"
#include "app_timer.h"
#include "app_touch_pads.h"
#include "app_wifi_station.h"


static const char *LOG_TAG = "Secure_Soil_Moisture";

static esp_event_loop_handle_t app_event_loop_handle;


//extern const uint8_t ca_cert_pem_start[] asm("_binary_mosq_ca_crt_start");
//extern const uint8_t ca_cert_pem_end[] asm("_binary_mosq_ca_crt_end");
extern const uint8_t server_cert_pem_start[] asm("_binary_mosq_server_crt_start");
extern const uint8_t server_cert_pem_end[] asm("_binary_mosq_server_crt_end");
extern const uint8_t client_cert_pem_start[] asm("_binary_mosq_client_crt_start");
extern const uint8_t client_cert_pem_end[] asm("_binary_mosq_client_crt_end");
extern const uint8_t client_key_pem_start[] asm("_binary_mosq_client_key_start");
extern const uint8_t client_key_pem_end[] asm("_binary_mosq_client_key_end");



/***
static void app_mqtt_start(void) {
    const esp_mqtt_client_config_t mqtt_cfg = {
        .event_handle = app_mqtt_event_handler,
        .uri = CONFIG_MQTT_BROKER_URL,
        .user_context = get_static_app_mqtt(),
        .client_cert_pem = (const char *)client_cert_pem_start,
        .client_key_pem = (const char *)client_key_pem_start,
    };

    ESP_LOGI(LOG_TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_start(client);
}


static void XXmqtt_app_start(void)
{
    const esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = "mqtts://test.mosquitto.org:8884",
        .broker.verification.certificate = (const char *)server_cert_pem_start,
        .credentials = {
            .authentication = {
                .certificate = (const char *)client_cert_pem_start,
                .key = (const char *)client_key_pem_start,
            },
        }
    };

    ESP_LOGI(TAG, "[APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    // The last argument may be used to pass data to the event handler, in this example mqtt_event_handler
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}
***/



void app_main(void)
{
    esp_err_t ret;

    ESP_LOGI(LOG_TAG, "[APP] Startup..");
    ESP_LOGI(LOG_TAG, "[APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
    ESP_LOGI(LOG_TAG, "[APP] IDF version: %s", esp_get_idf_version());

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("Secure_Soil_Moisture", ESP_LOG_VERBOSE);
    esp_log_level_set("app_event_loop", ESP_LOG_VERBOSE);
    esp_log_level_set("app_sntp_sync_time", ESP_LOG_VERBOSE);
    esp_log_level_set("app_timer", ESP_LOG_VERBOSE);
    esp_log_level_set("app_touch_pads", ESP_LOG_VERBOSE);
    esp_log_level_set("app_wifi_station", ESP_LOG_VERBOSE);

    esp_log_level_set("TRANSPORT_BASE", ESP_LOG_DEBUG);
    // esp_log_level_set("TRANSPORT_TCP", ESP_LOG_DEBUG);
    // esp_log_level_set("TRANSPORT_SSL", ESP_LOG_DEBUG);
    esp_log_level_set("TRANSPORT", ESP_LOG_DEBUG);
    esp_log_level_set("OUTBOX", ESP_LOG_DEBUG);

    //Initialize NVS
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(create_app_event_loop(&app_event_loop_handle));

    app_wifi_station_init();
    app_sntp_sync_time();
    app_timer_init(app_event_loop_handle);
    app_read_touch_pads_init(app_event_loop_handle);

    //TODO:...
    //app_mqtt_start();
}
