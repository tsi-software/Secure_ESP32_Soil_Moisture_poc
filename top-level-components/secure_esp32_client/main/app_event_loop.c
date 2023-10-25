/* app_event_loop.c

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "app_event_loop.h"


static const char* LOG_TAG = "app_event_loop";


esp_err_t create_app_event_loop(esp_event_loop_handle_t *event_loop_handle)
{
    ESP_LOGI(LOG_TAG, "Creating app event loop.");

    esp_event_loop_args_t event_loop_args = {
        .queue_size = 5,
        .task_name = "app_event_loop_task", // task will be created
        .task_priority = uxTaskPriorityGet(NULL),
        .task_stack_size = 3072,
        .task_core_id = tskNO_AFFINITY
    };

    esp_err_t err_result = esp_event_loop_create(&event_loop_args, event_loop_handle);
    //ESP_ERROR_CHECK(err_result);

    ESP_LOGI(LOG_TAG, "App event loop created.");
    return err_result;
}
