/*
app_mqtt50.c
*/

#include <stdio.h>
#include "freertos/FreeRTOS.h"
//#include "freertos/task.h"
#include "esp_log.h"
//#include "esp_system.h"
#include "mqtt_client.h"

#include "app_mqtt50.h"
#include "app_touch_pads.h"


static const char *LOG_TAG = "app_mqtt";


// extern const uint8_t ca_cert_pem_start[] asm("_binary_mosq_ca_crt_start");
// extern const uint8_t ca_cert_pem_end[] asm("_binary_mosq_ca_crt_end");
// extern const uint8_t client_cert_pem_start[] asm("_binary_mosq_client_crt_start");
// extern const uint8_t client_cert_pem_end[] asm("_binary_mosq_client_crt_end");
// extern const uint8_t client_key_pem_start[] asm("_binary_mosq_client_key_start");
// extern const uint8_t client_key_pem_end[] asm("_binary_mosq_client_key_end");



// static esp_mqtt5_user_property_item_t user_property_arr[] = {
//         {"board", "esp32"},
//         {"u", "user"},
//         {"p", "password"}
//     };
// #define USE_PROPERTY_ARR_SIZE   sizeof(user_property_arr)/sizeof(esp_mqtt5_user_property_item_t)

/***
static esp_mqtt5_publish_property_config_t publish_property = {
    .payload_format_indicator = 1,
    .message_expiry_interval = 1000,
    .topic_alias = 0,
    .response_topic = "/topic/test/response",
    .correlation_data = "123456",
    .correlation_data_len = 6,
};

static esp_mqtt5_subscribe_property_config_t subscribe_property = {
    .subscribe_id = 25555,
    .no_local_flag = false,
    .retain_as_published_flag = false,
    .retain_handle = 0,
    .is_share_subscribe = true,
    .share_name = "group1",
};

static esp_mqtt5_subscribe_property_config_t subscribe1_property = {
    .subscribe_id = 25555,
    .no_local_flag = true,
    .retain_as_published_flag = false,
    .retain_handle = 0,
};

static esp_mqtt5_unsubscribe_property_config_t unsubscribe_property = {
    .is_share_subscribe = true,
    .share_name = "group1",
};

static esp_mqtt5_disconnect_property_config_t disconnect_property = {
    .session_expiry_interval = 60,
    .disconnect_reason = 0,
};
***/



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
    esp_mqtt_event_handle_t event = event_data;
    // esp_mqtt_client_handle_t client = event->client;
    // int msg_id;

    ESP_LOGD(LOG_TAG, "free heap size is %" PRIu32 ", minimum %" PRIu32, esp_get_free_heap_size(), esp_get_minimum_free_heap_size());
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(LOG_TAG, "MQTT_EVENT_CONNECTED");

        /***
        print_user_property(event->property->user_property);
        esp_mqtt5_client_set_user_property(&publish_property.user_property, user_property_arr, USE_PROPERTY_ARR_SIZE);
        esp_mqtt5_client_set_publish_property(client, &publish_property);
        msg_id = esp_mqtt_client_publish(client, "/topic/qos1", "data_3", 0, 1, 1);
        esp_mqtt5_client_delete_user_property(publish_property.user_property);
        publish_property.user_property = NULL;
        ESP_LOGI(LOG_TAG, "sent publish successful, msg_id=%d", msg_id);

        esp_mqtt5_client_set_user_property(&subscribe_property.user_property, user_property_arr, USE_PROPERTY_ARR_SIZE);
        esp_mqtt5_client_set_subscribe_property(client, &subscribe_property);
        msg_id = esp_mqtt_client_subscribe(client, "/topic/qos0", 0);
        esp_mqtt5_client_delete_user_property(subscribe_property.user_property);
        subscribe_property.user_property = NULL;
        ESP_LOGI(LOG_TAG, "sent subscribe successful, msg_id=%d", msg_id);

        esp_mqtt5_client_set_user_property(&subscribe1_property.user_property, user_property_arr, USE_PROPERTY_ARR_SIZE);
        esp_mqtt5_client_set_subscribe_property(client, &subscribe1_property);
        msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 2);
        esp_mqtt5_client_delete_user_property(subscribe1_property.user_property);
        subscribe1_property.user_property = NULL;
        ESP_LOGI(LOG_TAG, "sent subscribe successful, msg_id=%d", msg_id);

        esp_mqtt5_client_set_user_property(&unsubscribe_property.user_property, user_property_arr, USE_PROPERTY_ARR_SIZE);
        esp_mqtt5_client_set_unsubscribe_property(client, &unsubscribe_property);
        msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos0");
        ESP_LOGI(LOG_TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
        esp_mqtt5_client_delete_user_property(unsubscribe_property.user_property);
        unsubscribe_property.user_property = NULL;
        ***/
        break;

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(LOG_TAG, "MQTT_EVENT_DISCONNECTED");
        // print_user_property(event->property->user_property);
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(LOG_TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        // print_user_property(event->property->user_property);
        // esp_mqtt5_client_set_publish_property(client, &publish_property);
        // msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
        // ESP_LOGI(LOG_TAG, "sent publish successful, msg_id=%d", msg_id);
        break;

    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(LOG_TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        // print_user_property(event->property->user_property);
        // esp_mqtt5_client_set_user_property(&disconnect_property.user_property, user_property_arr, USE_PROPERTY_ARR_SIZE);
        // esp_mqtt5_client_set_disconnect_property(client, &disconnect_property);
        // esp_mqtt5_client_delete_user_property(disconnect_property.user_property);
        // disconnect_property.user_property = NULL;
        // esp_mqtt_client_disconnect(client);
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
        ESP_LOGI(LOG_TAG, "MQTT_EVENT_ERROR");
        //print_user_property(event->property->user_property);
        ESP_LOGI(LOG_TAG, "MQTT5 return code is %d", event->error_handle->connect_return_code);
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


static void app_touch_value_handler(void* handler_args, esp_event_base_t base, int32_t id, void* event_data)
{
    app_touch_value_change_event_payload *payload = event_data;
    // typedef struct {
    //     uint16_t touch_value;
    //     uint8_t touch_pad_num;
    // } app_touch_value_change_event_payload;

    // MQTT Topic
    // soilmoisture/<device-id>/{analog,capacitive}/<sensor-id>
    // The Message is the sensor's numeric value formatted as a string.
    // touch_pad_num is 8 bits  ... 2^8 = 256 (i.e. 3 characters)
    // touch_value is 16 bits   ... 2^16 = 65536 (i.e. 5 characters)
    // utc_timestamp is 64 bits ... 2^64 ~ 18,446,744,073,709,600,000 (i.e. 20 characters)
    int num_of_characters;
    char topic[27 + 4 + 2];  // 2^8 needs 3 characters.
    char data[6 + 2 + 20 + 2];  //2^16 = needs 5 characters and 2^64 needs 20 characters.
    //TODO: test 'num_of_characters' and handle error situation as necessary.
    //TODO: implement a meaningful <device-id>.
    num_of_characters = snprintf(topic, sizeof(topic), "/soilmoisture/1/capacitive/%u", payload->touch_pad_num);
    num_of_characters = snprintf(data, sizeof(data), "%u,%lld", payload->touch_value, payload->utc_timestamp);

    // int esp_mqtt_client_enqueue(
    //     esp_mqtt_client_handle_t client,
    //     const char *topic,
    //     const char *data, int len,
    //     int qos, int retain, bool store
    // )
    esp_mqtt_client_handle_t mqtt_client = handler_args;
    esp_mqtt_client_enqueue(mqtt_client, topic, data,0, 0,0,true);

    ESP_LOGI(LOG_TAG, "MQTT ENQUEUED: %s, %s", topic, data);
}



void app_mqtt50_start(
        esp_event_loop_handle_t event_loop,
        const char *broker_url,
        const char *ca_cert,
        const char *client_cert,
        const char *client_key
) {
    esp_mqtt5_connection_property_config_t connect_property = {
        .session_expiry_interval = 0, //10,  // seconds
        .maximum_packet_size = 1024,
        .receive_maximum = 65535,
        .topic_alias_maximum = 2,
        .request_resp_info = true,
        .request_problem_info = true,
        .will_delay_interval = 10,
        .message_expiry_interval = 10,
        .payload_format_indicator = true,
        .response_topic = "/soilmoisture/response",
        .correlation_data = "soilmoisture",
        .correlation_data_len = 12,
    };

    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = broker_url,
        .broker.verification.certificate = ca_cert,
        .credentials = {
          .authentication = {
            .certificate = client_cert,
            .key = client_key,
          },
        },
        .session.protocol_ver = MQTT_PROTOCOL_V_5,
        // .session.last_will.topic = "/topic/will",
        // .session.last_will.msg = "i will leave",
        // .session.last_will.msg_len = 12,
        // .session.last_will.qos = 1,
        // .session.last_will.retain = true,
        //.network.disable_auto_reconnect = true,
    };

    esp_err_t err;
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);

    // Set connection properties and user properties 
    ////esp_mqtt5_client_set_user_property(&connect_property.user_property, user_property_arr, USE_PROPERTY_ARR_SIZE);
    ////esp_mqtt5_client_set_user_property(&connect_property.will_user_property, user_property_arr, USE_PROPERTY_ARR_SIZE);
    esp_mqtt5_client_set_connect_property(client, &connect_property);

    // If you call esp_mqtt5_client_set_user_property to set user properties, DO NOT forget to delete them.
    // esp_mqtt5_client_set_connect_property will malloc buffer to store the user_property and you can delete it after
    ////esp_mqtt5_client_delete_user_property(connect_property.user_property);
    ////esp_mqtt5_client_delete_user_property(connect_property.will_user_property);

    // The last argument may be used to pass data to the event handler, in this example mqtt_event_handler 
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt5_event_handler, NULL);
    err = esp_mqtt_client_start(client);
    switch(err) {
    case ESP_OK:
        ESP_LOGI(LOG_TAG, "Connecting to MQTT5 server '%s'.", mqtt_cfg.broker.address.uri);
        break;
    case ESP_ERR_INVALID_ARG:
        ESP_LOGE(LOG_TAG, "MQTT5 Invalid Arg (%s) - Server '%s'!", esp_err_to_name(err), mqtt_cfg.broker.address.uri);
        break;
    case ESP_FAIL:
    default:
        ESP_LOGE(LOG_TAG, "MQTT5 Error (%s) - Server '%s'!", esp_err_to_name(err), mqtt_cfg.broker.address.uri);
        break;
    }


    // WARNING: NOT FOR PRODUCTION!!  THIS LOG STATEMENT REVEALS SENSITIVE DATA!
    //ESP_LOGW(LOG_TAG, "MQTT CONFIG:\n%s\n%s\n%s\n%s\n", broker_url, ca_cert, client_cert, client_key);


    // esp_err_t esp_event_handler_instance_register_with(
    //     esp_event_loop_handle_t event_loop,
    //     esp_event_base_t event_base,
    //     int32_t event_id,
    //     esp_event_handler_t event_handler,
    //     void *event_handler_arg,
    //     esp_event_handler_instance_t *instance
    // )
    ESP_ERROR_CHECK(esp_event_handler_instance_register_with(
            event_loop,
            APP_TOUCH_EVENTS,
            APP_TOUCH_VALUE_CHANGE_EVENT,
            app_touch_value_handler,
            client,
            NULL
    ));
}
