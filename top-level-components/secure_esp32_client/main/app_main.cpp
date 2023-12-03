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


//------------------------------------------------------------------------------
// JUST TESTING!
// Forward Declarations.
static void testTask1(void *pvParameters);
static void testTask2(void *pvParameters);

struct TestTaskParams {
    TaskHandle_t xParentHandle;
    UBaseType_t xNotificationIndex;
};
TestTaskParams testTaskParams1, testTaskParams2;
//------------------------------------------------------------------------------



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
        app_wifi_station_init(
                wifiConfig.get_ssid(),
                wifiConfig.get_password()
        );
    }
    {
        GlobalConfig globalConfig;
        app_sntp_sync_time( globalConfig.get_sntp_server() );
    }


    // JUST TESTING!
    UBaseType_t currentPriority = uxTaskPriorityGet(xTaskGetCurrentTaskHandle());

    // TODO: throw a compile time error if CONFIG_FREERTOS_TASK_NOTIFICATION_ARRAY_ENTRIES is less than 3 in "sdkconfig"
    // see
    //   https://www.freertos.org/a00110.html#configUSE_TASK_NOTIFICATIONS
    //   https://www.freertos.org/xTaskNotifyGive.html
    //   https://www.freertos.org/ulTaskNotifyTake.html

    TaskHandle_t xTaskHandle1 = NULL;
    testTaskParams1.xParentHandle = xTaskGetCurrentTaskHandle();
    testTaskParams1.xNotificationIndex = 1;

    TaskHandle_t xTaskHandle2 = NULL;
    testTaskParams2.xParentHandle = xTaskGetCurrentTaskHandle();
    testTaskParams2.xNotificationIndex = 2;

    BaseType_t createErr;
    ESP_LOGE(LOG_TAG, "Creating testTask1.");
    createErr = xTaskCreate(testTask1, "testTask1", 4096, &testTaskParams1, currentPriority, &xTaskHandle1);

    ESP_LOGE(LOG_TAG, "Main Task - waiting for 5 seconds.");
    vTaskDelay(5000/portTICK_PERIOD_MS);

    ESP_LOGE(LOG_TAG, "WAITING for testTask1.");
    ulTaskNotifyTakeIndexed(testTaskParams1.xNotificationIndex, pdTRUE, (TickType_t)0xffff);
    ESP_LOGE(LOG_TAG, "testTask1 DONE WAITING.");

    // ESP_LOGE(LOG_TAG, "WAITING for testTask2.");
    // ulTaskNotifyTakeIndexed(testTaskParams2.xNotificationIndex, pdTRUE, (TickType_t)0xffff);
    // ESP_LOGE(LOG_TAG, "testTask2 DONE WAITING.");


    MqttConfig mqttConfig; //JUST TESTING.
    {
        //JUST TESTING.
        //MqttConfig mqttConfig;
        app_mqtt50_start(
                app_event_loop_handle,
                mqttConfig.get_broker_url(),
                mqttConfig.get_ca_cert(),
                mqttConfig.get_client_cert(),
                mqttConfig.get_client_key()
        );
    }


    ESP_LOGE(LOG_TAG, "Creating testTask2.");
    createErr = xTaskCreate(testTask2, "testTask2", 4096, &testTaskParams2, currentPriority, &xTaskHandle2);
    ESP_LOGE(LOG_TAG, "WAITING for testTask2.");
    ulTaskNotifyTakeIndexed(testTaskParams2.xNotificationIndex, pdTRUE, (TickType_t)0xffff);
    ESP_LOGE(LOG_TAG, "testTask2 DONE WAITING.");

    app_timer_init(app_event_loop_handle);
    app_read_touch_pads_init(app_event_loop_handle);
}



static void testTask1(void *pvParameters)
{
    TestTaskParams *params = (TestTaskParams *)pvParameters;

    ESP_LOGE(LOG_TAG, "testTask1 - about to GIVE.");
    //BaseType_t xTaskNotifyGiveIndexed( TaskHandle_t xTaskToNotify, UBaseType_t uxIndexToNotify );
    xTaskNotifyGiveIndexed(params->xParentHandle, params->xNotificationIndex);
    ESP_LOGE(LOG_TAG, "testTask1 - GAVE.");

    vTaskDelete( NULL );
}



static void testTask2(void *pvParameters)
{
    TestTaskParams *params = (TestTaskParams *)pvParameters;

    ESP_LOGE(LOG_TAG, "testTask2 - waiting for 5 seconds.");
    const TickType_t xDelay = 5000 / portTICK_PERIOD_MS;
    vTaskDelay(xDelay);

    ESP_LOGE(LOG_TAG, "testTask2 - about to GIVE.");
    //BaseType_t xTaskNotifyGiveIndexed( TaskHandle_t xTaskToNotify, UBaseType_t uxIndexToNotify );
    xTaskNotifyGiveIndexed(params->xParentHandle, params->xNotificationIndex);
    ESP_LOGE(LOG_TAG, "testTask2 - GAVE.");

    vTaskDelete( NULL );
}
