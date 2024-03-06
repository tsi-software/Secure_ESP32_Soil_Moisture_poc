/*
app_touch_pads.h
*/

#ifndef _APP_TOUCH_PADS_H_
#define _APP_TOUCH_PADS_H_


#include <time.h>
#include "app_events.h"


#ifdef __cplusplus
extern "C" {
#endif

//---------------------------------
// APP_TOUCH_EVENTS
// are defined in app_events.h
//---------------------------------

extern void app_read_touch_pads_init(esp_event_loop_handle_t event_loop);

#ifdef __cplusplus
}
#endif


#endif // _APP_TOUCH_PADS_H_
