/*
app_timer.h
*/

#ifndef _APP_TIMER_H_
#define _APP_TIMER_H_


#include "app_event_loop.h"


#ifdef __cplusplus
extern "C" {
#endif


ESP_EVENT_DECLARE_BASE(APP_TIMER_EVENTS);

enum {
    APP_TIMER_TICK_EVENT
};

extern void app_timer_init(esp_event_loop_handle_t event_loop_handle);


#ifdef __cplusplus
}
#endif


#endif // _APP_TIMER_H_
