/*
app_mqtt50.h
*/

#ifndef _APP_MQTT50_H_
#define _APP_MQTT50_H_

#include "mqtt_client.h"
#include "app_globals.h"
#include "app_event_loop.h"


#ifdef __cplusplus
extern "C" {
#endif

// function defined in app_mqtt50_init.c
extern esp_mqtt_client_handle_t app_mqtt50_init(
        const char *broker_url,
        const char *ca_cert,
        const char *client_cert,
        const char *client_key
);

// function defined in app_mqtt50.cpp
extern void app_mqtt50_start(
        globalTaskNotifyParams *startup_notify,
        esp_event_loop_handle_t event_loop,
        const char *device_id,
        const char *broker_url,
        esp_mqtt_client_handle_t client
);

#ifdef __cplusplus
}
#endif



#ifdef __cplusplus
#include <map>
#include <string>

class MqttUserProperties {
public:
    using properties_map = std::map<std::string, std::string>;

    MqttUserProperties()
    { }

    MqttUserProperties(mqtt5_user_property_handle_t user_property) {
        if (!user_property) {
            //TODO: log an appropriate message.
            return;
        }

        uint8_t prop_count = esp_mqtt5_client_get_user_property_count(user_property);
        if (!prop_count) {
            //TODO: log an appropriate message.
            return;
        }

        esp_mqtt5_user_property_item_t *user_property_item;
        user_property_item = static_cast<esp_mqtt5_user_property_item_t *>(
                malloc(prop_count * sizeof(esp_mqtt5_user_property_item_t))
        );
        if (!user_property_item) {
            //TODO: log an appropriate message.
            return;
        }

        esp_err_t err = esp_mqtt5_client_get_user_property(user_property, user_property_item, &prop_count);
        if (err == ESP_OK) {
            for (uint8_t i = 0; i < prop_count; ++i) {
                const char *key = user_property_item[i].key;
                const char *value = user_property_item[i].value;
                if (key && *key && value && *value) {
                    user_properties.insert({key, value});
                }
                if (key) {
                    free((void*)key);
                }
                if (value) {
                    free((void*)value);
                }
                // esp_mqtt5_user_property_item_t *t = &user_property_item[i];
                // user_properties.insert({t->key, t->value});
                // if (t->key) {
                //     free((void*)t->key);
                // }
                // if (t->value) {
                //     free((void*)t->value);
                // }
            }
        } else {
            // err = ESP_FAIL or ESP_ERR_NO_MEM
            //TODO: log an appropriate message.
            // make sure 'user_property_item' get freed just below!
        }
        free(user_property_item);
    }

    MqttUserProperties(const MqttUserProperties& other) : user_properties(other.user_properties)
    { }

    MqttUserProperties(MqttUserProperties&& other) noexcept : user_properties(other.user_properties)
    { }

    virtual ~MqttUserProperties()
    { }


    // see https://en.cppreference.com/w/cpp/language/operators
    MqttUserProperties& operator=(const MqttUserProperties& other) {
        if (this == &other) {
            return *this;
        }
        user_properties = other.user_properties;
        return *this;
    }

    MqttUserProperties& operator=(MqttUserProperties&& other) {
        if (this == &other) {
            return *this;
        }
        user_properties = other.user_properties;
        //user_properties = std::move(other.user_properties);
        return *this;
    }


    const properties_map& get_user_properties() {
        return user_properties;
    }

    properties_map::iterator find(const properties_map::key_type& key) {
        return user_properties.find(key);
    }

    bool exists(const properties_map::key_type& key) {
        return user_properties.find(key) != user_properties.end();
    }

    properties_map::mapped_type& operator[](const properties_map::key_type& key) {
        return user_properties[key];
    }

    // properties_map::mapped_type& operator[](properties_map::key_type&& key) {
    //     return user_properties[key];
    // }

private:
    properties_map user_properties;
};
#endif // __cplusplus



#endif // _APP_MQTT50_H_
