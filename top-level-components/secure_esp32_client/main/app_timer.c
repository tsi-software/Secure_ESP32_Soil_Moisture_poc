/*
app_timer.c
*/

#include <stdio.h>
#include <string.h>
//#include <unistd.h>
#include "esp_timer.h"
#include "esp_log.h"
//#include "esp_sleep.h"
#include "sdkconfig.h"

#include "app_timer.h"


static const char* LOG_TAG = "app_timer";


ESP_EVENT_DEFINE_BASE(APP_TIMER_EVENTS);


static void periodic_timer_callback(void* arg)
{
    int64_t time_since_boot = esp_timer_get_time();
    ESP_LOGV(LOG_TAG, "Periodic timer called, time since boot: %lld us", time_since_boot);

    ESP_ERROR_CHECK(esp_event_post_to(
            (esp_event_loop_handle_t)arg,
            APP_TIMER_EVENTS, APP_TIMER_TICK_EVENT,
            &time_since_boot, sizeof(time_since_boot),
            portMAX_DELAY
    ));
}


void app_timer_init(esp_event_loop_handle_t event_loop_handle)
{
    const esp_timer_create_args_t periodic_timer_args = {
            .callback = &periodic_timer_callback,
            .arg = (void *)event_loop_handle,
            .name = "app_timer"
    };

    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));

    // Start the timer for every 1 second.
    //ESP_LOGW(LOG_TAG, "App Timer NEEDS TO BE STARTED!");
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 1000000));
    ESP_LOGI(LOG_TAG, "App Timer Started");
}
