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



static void periodic_timer_callback(void* arg)
{
    int64_t time_since_boot = esp_timer_get_time();
    ESP_LOGI(LOG_TAG, "Periodic timer called, time since boot: %lld us", time_since_boot);
}


void app_timer_init(void)
{
    const esp_timer_create_args_t periodic_timer_args = {
            .callback = &periodic_timer_callback,
            //.arg = (void *)event_queue,
            .name = "app_timer"
    };

    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));

    // Start the timer for every 1 second.
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 1000000));
    ESP_LOGI(LOG_TAG, "App Timer Started");
}
