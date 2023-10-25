/*
app_touch_pads.c
*/

#include "freertos/FreeRTOS.h"
//#include "freertos/task.h"
#include "esp_log.h"



#include "app_timer.h"
#include "app_touch_pads.h"


static const char* LOG_TAG = "app_touch_pads";


static void app_timer_tick_handler(void* handler_args, esp_event_base_t base, int32_t id, void* event_data)
{
    ESP_LOGI(LOG_TAG, "App Touch Pad timer tick handler called.");
}


void app_read_touch_pads_init(esp_event_loop_handle_t event_loop_handle)
{
    ESP_ERROR_CHECK(esp_event_handler_instance_register_with(
            event_loop_handle,
            APP_TIMER_EVENTS, APP_TIMER_TICK_EVENT,
            app_timer_tick_handler,
            event_loop_handle,
            NULL
    ));
}
