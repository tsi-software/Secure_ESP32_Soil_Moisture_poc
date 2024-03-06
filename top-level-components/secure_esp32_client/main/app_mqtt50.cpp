/*
app_mqtt50.cpp
*/

#include <sstream>

#include "freertos/FreeRTOS.h"
//#include "freertos/task.h"
#include "esp_log.h"
//#include "esp_system.h"
#include "mqtt_client.h"

#include "app_mqtt50.h"
#include "app_touch_pads.h"


static const char *LOG_TAG = "app_mqtt";

static const esp_mqtt_event_id_t APP_EVENT_ANY_ID = static_cast<esp_mqtt_event_id_t>(ESP_EVENT_ANY_ID);

struct mqtt_publish_params {
    esp_mqtt_client_handle_t mqtt_client;
    const char *device_id;
};



static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(LOG_TAG, "Last error %s: 0x%x", message, error_code);
    }
}



// static void print_user_property(mqtt5_user_property_handle_t user_property)
// {
//     if (user_property) {
//         uint8_t count = esp_mqtt5_client_get_user_property_count(user_property);
//         if (count) {
//             esp_mqtt5_user_property_item_t *item = malloc(count * sizeof(esp_mqtt5_user_property_item_t));
//             if (esp_mqtt5_client_get_user_property(user_property, item, &count) == ESP_OK) {
//                 for (int i = 0; i < count; i ++) {
//                     esp_mqtt5_user_property_item_t *t = &item[i];
//                     ESP_LOGI(LOG_TAG, "key is %s, value is %s", t->key, t->value);
//                     free((char *)t->key);
//                     free((char *)t->value);
//                 }
//             }
//             free(item);
//         }
//     }
// }



/**
TODO: mqtt_startup_handler(...) and mqtt5_event_handler(...) are redundant.
      roll mqtt_startup_handler() into mqtt5_event_handler().

The only purpose of mqtt_startup_handler is to Notify the main thread that this task
has either successfully started or errored out.
In either case, after this point, this handler is no longer needed.
*/
static void mqtt_startup_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(LOG_TAG, "mqtt_startup_handler: Event dispatched from event loop base=%s, event_id=%" PRIi32, base, event_id);

    if (!( (esp_mqtt_event_id_t)event_id==MQTT_EVENT_CONNECTED || (esp_mqtt_event_id_t)event_id==MQTT_EVENT_ERROR )) {
        // We are only interested in the Connected or Error events here.
        return;
    }

    // The only purpose of mqtt_startup_handler is to Notify the main thread that this task
    //  has either successfully started or errored out.
    // In either case, after this point, this handler is no longer needed.
    esp_err_t err;
    esp_mqtt_event_handle_t event = static_cast<esp_mqtt_event_handle_t>(event_data);
    err = esp_mqtt_client_unregister_event(event->client, APP_EVENT_ANY_ID, mqtt_startup_handler);
    if (err != ESP_OK) {
        ESP_LOGE(LOG_TAG, "esp_mqtt_client_unregister_event(...,mqtt_startup_handler): %s!", esp_err_to_name(err));
    }

    globalTaskNotifyParams *startup_notify = (globalTaskNotifyParams *)handler_args;

    // Notify the main task that this task will no longer be accessing the memory allocated to the config info.
    xTaskNotifyGiveIndexed(startup_notify->taskToNotify, startup_notify->indexToNotify);
}



/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
static void mqtt5_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(LOG_TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32, base, event_id);
    ESP_LOGD(LOG_TAG, "free heap size is %" PRIu32 ", minimum %" PRIu32,
             esp_get_free_heap_size(),
             esp_get_minimum_free_heap_size()
    );

    esp_mqtt_event_handle_t event = static_cast<esp_mqtt_event_handle_t>(event_data);

    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_BEFORE_CONNECT:
        ESP_LOGD(LOG_TAG, "MQTT_EVENT_BEFORE_CONNECT");
        break;

    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(LOG_TAG, "MQTT_EVENT_CONNECTED");
        break;

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(LOG_TAG, "MQTT_EVENT_DISCONNECTED");
        // print_user_property(event->property->user_property);
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGD(LOG_TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGD(LOG_TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(LOG_TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        // print_user_property(event->property->user_property);
        break;

    case MQTT_EVENT_DATA:
        ESP_LOGI(LOG_TAG, "MQTT_EVENT_DATA");
        // print_user_property(event->property->user_property);
        ESP_LOGI(LOG_TAG, "payload_format_indicator is %d", event->property->payload_format_indicator);
        ESP_LOGI(LOG_TAG, "response_topic is %.*s", event->property->response_topic_len, event->property->response_topic);
        ESP_LOGI(LOG_TAG, "correlation_data is %.*s", event->property->correlation_data_len, event->property->correlation_data);
        ESP_LOGI(LOG_TAG, "content_type is %.*s", event->property->content_type_len, event->property->content_type);
        ESP_LOGI(LOG_TAG, "TOPIC=%.*s", event->topic_len, event->topic);
        ESP_LOGI(LOG_TAG, "DATA=%.*s", event->data_len, event->data);
        break;

    case MQTT_EVENT_ERROR:
        ESP_LOGW(LOG_TAG, "MQTT_EVENT_ERROR");
        //print_user_property(event->property->user_property);
        ESP_LOGW(LOG_TAG, "MQTT5 return code is %d", event->error_handle->connect_return_code);
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(LOG_TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;

    default:
        ESP_LOGI(LOG_TAG, "Other event id:%d", event->event_id);
        break;
    }
}



/*
Handle Touch Pad messages coming from the app queue
and send them out as MQTT messages.
*/
static void app_touch_value_handler(void* handler_args, esp_event_base_t base, int32_t id, void* event_data)
{
    struct mqtt_publish_params *mqtt_publish_params = static_cast<struct mqtt_publish_params *>(handler_args);
    app_touch_value_change_event_payload *payload = static_cast<app_touch_value_change_event_payload *>(event_data);

    const char *topic_str_fmt = "soilmoisture/%s/touchpad/%u";
    const char *data_str_fmt =  "%lu,%lld";

    // MQTT Topic
    // soilmoisture/<device-id>/{analog,touchpad}/<sensor-id>
    // The Message is the sensor's numeric value formatted as a string.
    // touch_pad_num is 8 bits  ... 2^8 = 256 (i.e. 3 characters)
    // MQTT Data
    // touch_value if 16 bits   ... 2^16 = 65536 (i.e. 5 characters)
    // touch_value if 32 bits   ... 2^32 = 4294967296 (i.e. 10 characters)
    // utc_timestamp is 64 bits ... 2^64 ~ 18,446,744,073,709,600,000 (i.e. 20 characters)
    const unsigned device_id_strlen = strlen(mqtt_publish_params->device_id);
    const unsigned touch_pad_num_strlen = 3;
    const unsigned touch_value_strlen = 10;
    const unsigned timestamp_strlen = 20;

    // Calculate the length of each formatted string,
    //... and always add 1 for the null terminator.
    const unsigned topic_strlen = strlen(topic_str_fmt)-4 + device_id_strlen + touch_pad_num_strlen + 1;
    const unsigned data_strlen = strlen(data_str_fmt)-6 + touch_value_strlen + timestamp_strlen + 1;

    char topic[topic_strlen];
    char data[data_strlen];
    int num_of_characters;
    num_of_characters = snprintf(topic, topic_strlen, topic_str_fmt,
                                 mqtt_publish_params->device_id, payload->touch_pad_num);
    num_of_characters = snprintf(data, data_strlen, data_str_fmt,
                                 payload->touch_value, payload->utc_timestamp);

    //TODO: test 'num_of_characters' and handle error situation as necessary.

    // https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/protocols/mqtt.html#_CPPv416esp_mqtt_event_t
    // int esp_mqtt_client_enqueue(
    //     esp_mqtt_client_handle_t client,
    //     const char *topic,
    //     const char *data, int len,
    //     int qos, int retain, bool store
    // )
    // Returns message_id if queued successfully, -1 on failure, -2 in case of full outbox.
    int msg_id = esp_mqtt_client_enqueue(mqtt_publish_params->mqtt_client, topic, data,0, 0,0,true);
    if (msg_id == -1) {
        // Failure.
        ESP_LOGE(LOG_TAG, "FAILURE: esp_mqtt_client_enqueue(): %s, %s", topic, data);
    } else if (msg_id == -2) {
        // Outbox Full.
        ESP_LOGE(LOG_TAG, "OUTBOX FULL: esp_mqtt_client_enqueue(): %s, %s", topic, data);
    } else {
        ESP_LOGD(LOG_TAG, "MQTT ENQUEUED: msg_id:%d, %s, %s", msg_id, topic, data);
    }
}



/*
  Important:
    the memory allocated to 'startup_notify' must not be released immediately
    because it may be referred to at any time in other tasks.
*/
void app_mqtt50_start(
        globalTaskNotifyParams *startup_notify,
        esp_event_loop_handle_t event_loop,
        const char *device_id,
        const char *broker_url,
        esp_mqtt_client_handle_t client
) {
    esp_err_t err;

    // The mqtt_startup_handler is only used to Notify the main thread that the mqtt task
    //  has either successfully started or errored out and that this task will no longer
    //  be accessing the memory allocated to the config arguments pass in to this function.
    err = esp_mqtt_client_register_event(client, APP_EVENT_ANY_ID, mqtt_startup_handler, startup_notify);
    if (err != ESP_OK) {
        ESP_LOGE(LOG_TAG, "esp_mqtt_client_register_event(...,mqtt_startup_handler): %s!", esp_err_to_name(err));
    }

    // The mqtt5_event_handler is used for general MQTT events.
    err = esp_mqtt_client_register_event(client, APP_EVENT_ANY_ID, mqtt5_event_handler, NULL);
    if (err != ESP_OK) {
        ESP_LOGE(LOG_TAG, "esp_mqtt_client_register_event(...,mqtt5_event_handler): %s!", esp_err_to_name(err));
    }

    err = esp_mqtt_client_start(client);
    if (err != ESP_OK) {
        // MQTT Client failed to start!
        esp_err_t err2;
        err2 = esp_mqtt_client_unregister_event(client, APP_EVENT_ANY_ID, mqtt_startup_handler);
        if (err2 != ESP_OK) {
            ESP_LOGE(LOG_TAG, "esp_mqtt_client_unregister_event(...,mqtt_startup_handler): %s!", esp_err_to_name(err2));
        }

        err2 = esp_mqtt_client_unregister_event(client, APP_EVENT_ANY_ID, mqtt5_event_handler);
        if (err2 != ESP_OK) {
            ESP_LOGE(LOG_TAG, "esp_mqtt_client_unregister_event(...,mqtt5_event_handler): %s!", esp_err_to_name(err2));
        }

        // Notify the main task that this task will no longer be accessing the memory allocated to the config arguments.
        xTaskNotifyGiveIndexed(startup_notify->taskToNotify, startup_notify->indexToNotify);
    }

    switch(err) {
    case ESP_OK:
        ESP_LOGI(LOG_TAG, "Connecting to MQTT5 server '%s'.", broker_url);
        break;
    case ESP_ERR_INVALID_ARG:
        ESP_LOGE(LOG_TAG, "MQTT5 Invalid Arg (%s) - Server '%s'!", esp_err_to_name(err), broker_url);
        return;
    case ESP_FAIL:
    default:
        ESP_LOGE(LOG_TAG, "MQTT5 Error (%s) - Server '%s'!", esp_err_to_name(err), broker_url);
        return;
    }

    //-------------------------------------------------------------------
    // Subscribe to MQTT Topics.
    //-------------------------------------------------------------------
    {
        /****
        // Use this scope to manage larger stack variables.
        const char *topic_str_fmt = "soilmoisture/%s/touchpad/config/#";

        // Calculate the length of formatted strings,
        //... and always add 1 for the null terminator.
        const unsigned topic_strlen = strlen(topic_str_fmt)-2 + device_id_strlen + 1;

        char topic[topic_strlen];
        snprintf(topic, topic_strlen, topic_str_fmt, device_id);
        ****/

        std::ostringstream sstr;
        sstr << "soilmoisture/" << device_id << "/touchpad/config/#";
        std::string topic_str = sstr.str();

        //TODO: make 'qos' a configurable value.
        const int qos = 0;
        int rslt = esp_mqtt_client_subscribe(client, topic_str.c_str(), qos);
        if (rslt >= 0) {
            ESP_LOGI(LOG_TAG, "Subscribed to '%s'.", topic_str.c_str());
        } else {
            ESP_LOGI(LOG_TAG, "FAILED to subscribe to '%s'.", topic_str.c_str());
        }
    }

    //-------------------------------------------------------------------
    // Start listening for Touch Pad messages coming from the app queue.
    //-------------------------------------------------------------------
    const size_t device_id_strlen = strlen(device_id);
    char *buffer = static_cast<char *>(malloc(device_id_strlen + 1));
    strncpy(buffer, device_id, device_id_strlen);
    buffer[device_id_strlen] = '\0';

    // 'mqtt_publish_params' must be static because it must survive after this function returns.
    static struct mqtt_publish_params mqtt_publish_params;
    mqtt_publish_params.mqtt_client = client;
    mqtt_publish_params.device_id = buffer;

    ESP_ERROR_CHECK(esp_event_handler_instance_register_with(
            event_loop,
            APP_TOUCH_EVENTS,
            APP_TOUCH_VALUE_CHANGE_EVENT,
            app_touch_value_handler,
            &mqtt_publish_params,
            NULL
    ));
}
