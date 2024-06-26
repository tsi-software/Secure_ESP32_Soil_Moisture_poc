/*
app_events.h
*/

#ifndef _APP_EVENTS_H_
#define _APP_EVENTS_H_


#ifdef __cplusplus
extern "C" {
#endif


//------------------------------------------------------------------------------
// APP_TOUCH_EVENTS
//------------------------------------------------------------------------------
ESP_EVENT_DECLARE_BASE(APP_TOUCH_EVENTS);
enum {
    // Events generated by and sent from the "Touch Pads" module.
    // DEPRECATED
    APP_TOUCH_VALUE_CHANGE_EVENT,

    // Events to be sent to the "Touch Pads" module.
    APP_TOUCH_FORCE_UPDATE,  //TODO: needs to be implemented.
};

typedef struct {
    time_t utc_timestamp;
    uint32_t touch_value;
    uint8_t touch_pad_num;
} app_touch_value_change_event_payload;



//------------------------------------------------------------------------------
// APP_MQTT_EVENTS
//------------------------------------------------------------------------------
ESP_EVENT_DECLARE_BASE(APP_MQTT_EVENTS);
enum {
    // Events generated by 
    APP_MQTT_xxx_EVENT
};



//------------------------------------------------------------------------------
// ???
//------------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif

#endif // _APP_EVENTS_H_
