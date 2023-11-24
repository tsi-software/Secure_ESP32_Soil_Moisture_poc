/*
app_event_loop.h
*/

#ifndef _APP_EVENT_LOOP_H_
#define _APP_EVENT_LOOP_H_


#include "esp_check.h"
#include "esp_event.h"
#include "esp_event_base.h"


#ifdef __cplusplus
extern "C" {
#endif

extern esp_err_t create_app_event_loop(esp_event_loop_handle_t *event_loop_handle);

#ifdef __cplusplus
}
#endif


#endif // _APP_EVENT_LOOP_H_
