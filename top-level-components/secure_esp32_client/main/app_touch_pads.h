/*
app_touch_pads.h
*/

#ifndef _APP_TOUCH_PADS_H_
#define _APP_TOUCH_PADS_H_


#include <time.h>
#include "app_event_loop.h"


#ifdef __cplusplus
extern "C" {
#endif


ESP_EVENT_DECLARE_BASE(APP_TOUCH_EVENTS);

enum {
    APP_TOUCH_VALUE_CHANGE_EVENT,
    APP_TOUCH_FORCE_UPDATE  //TODO: needs to be implemented.
};

typedef struct {
    time_t utc_timestamp;
    uint32_t touch_value;
    uint8_t touch_pad_num;
} app_touch_value_change_event_payload;

extern void app_read_touch_pads_init(esp_event_loop_handle_t event_loop);


#ifdef __cplusplus
}
#endif


#endif // _APP_TOUCH_PADS_H_
