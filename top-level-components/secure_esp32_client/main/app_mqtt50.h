/*
app_mqtt50.h
*/

#ifndef _APP_MQTT50_H_
#define _APP_MQTT50_H_

#include "app_globals.h"
#include "app_event_loop.h"


#ifdef __cplusplus
extern "C" {
#endif

extern void app_mqtt50_start(
        globalTaskNotifyParams *startup_notify,
        esp_event_loop_handle_t event_loop,
        const char *device_id,
        const char *broker_url,
        const char *ca_cert,
        const char *client_cert,
        const char *client_key
);

#ifdef __cplusplus
}
#endif


#endif // _APP_MQTT50_H_
