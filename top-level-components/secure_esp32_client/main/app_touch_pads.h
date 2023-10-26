/*
app_touch_pads.h
*/

#ifndef _APP_TOUCH_PADS_H_
#define _APP_TOUCH_PADS_H_


#include "app_event_loop.h"


#ifdef __cplusplus
extern "C" {
#endif

ESP_EVENT_DECLARE_BASE(APP_TOUCH_EVENTS);

enum {
    APP_TOUCH_VALUE_CHANGE_EVENT, //TODO: needs to be implemented.
    APP_TOUCH_FORCE_UPDATE        //TODO: needs to be implemented.
};

#ifdef __cplusplus
}
#endif


extern void app_read_touch_pads_init(esp_event_loop_handle_t event_loop_handle);


#endif // _APP_TOUCH_PADS_H_
