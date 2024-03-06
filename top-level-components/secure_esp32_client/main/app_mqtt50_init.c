/*
app_mqtt50_init.c
*/

//#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "mqtt_client.h"

#include "app_mqtt50.h"


static const char *LOG_TAG = "app_mqtt_init";



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



esp_mqtt_client_handle_t app_mqtt50_init(
        const char *broker_url,
        const char *ca_cert,
        const char *client_cert,
        const char *client_key
) {
    ESP_LOGD(LOG_TAG, "app_mqtt50_init(...)");

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
        .response_topic = "soilmoisture/response",
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

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);

    // Set connection properties and user properties 
    ////esp_mqtt5_client_set_user_property(&connect_property.user_property, user_property_arr, USE_PROPERTY_ARR_SIZE);
    ////esp_mqtt5_client_set_user_property(&connect_property.will_user_property, user_property_arr, USE_PROPERTY_ARR_SIZE);
    esp_mqtt5_client_set_connect_property(client, &connect_property);

    // If you call esp_mqtt5_client_set_user_property to set user properties, DO NOT forget to delete them.
    // esp_mqtt5_client_set_connect_property will malloc buffer to store the user_property and you can delete it after
    ////esp_mqtt5_client_delete_user_property(connect_property.user_property);
    ////esp_mqtt5_client_delete_user_property(connect_property.will_user_property);

    return client;
}
