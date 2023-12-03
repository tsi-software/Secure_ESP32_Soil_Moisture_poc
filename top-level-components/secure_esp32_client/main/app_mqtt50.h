/*
app_mqtt50.h
*/

#ifndef _APP_MQTT_H_
#define _APP_MQTT_H_


#include "app_event_loop.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void app_mqtt50_start(
        esp_event_loop_handle_t event_loop,
        const char *broker_url,
        const char *ca_cert,
        const char *client_cert,
        const char *client_key
);

#ifdef __cplusplus
}
#endif


#endif // _APP_MQTT_H_
