/*
app_config.cpp
*/

//#include "nvs_flash.h"
//#include "nvs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"

#include "app_config.hpp"

static const char *LOG_TAG = "app_config";



//------------------------------------------------------------------------------
// AppConfig
//------------------------------------------------------------------------------
AppConfig::AppConfig(const char *nvs_namespace) : nvs_namespace(nvs_namespace) {
    esp_err_t err = ESP_OK;
    // nvs_handle is automatically closed on desctruction.
    nvs_handle = nvs::open_nvs_handle(nvs_namespace, NVS_READONLY, &err);
    if (err != ESP_OK) {
        ESP_LOGW(LOG_TAG, "Error (%s) opening NVS handle for namespace '%s'!",
                 esp_err_to_name(err), nvs_namespace);
    }
}


/**
 * The caller of this function is responsible for releasing the memory
 *  allocated and returned by this function.
 */
char *AppConfig::get_str(const char *key)
{
    esp_err_t err;

    // Read the size of memory space required.
    size_t required_size = 0;  // value will default to 0, if not set yet in NVS
    err = nvs_handle->get_item_size(nvs::ItemType::SZ, key, required_size);
    if (err != ESP_OK) {
        if (err == ESP_ERR_NVS_NOT_FOUND) {
            ESP_LOGW(LOG_TAG, "Warning (%s) - KEY NOT FOUND: '%s'",
                     esp_err_to_name(err), key);
        } else {
            ESP_LOGE(LOG_TAG, "Error (%s) - KEY: '%s'!",
                     esp_err_to_name(err), key);
        }
        return nullptr;
    }

    // Read previously saved value if available.
    char *buffer = nullptr;
    if (required_size > 0) {
        // The caller of this function is responsible for releasing the memory allocated to 'buffer'.
        // Add 1 to guarantee a null terminator.
        buffer = new char[required_size + sizeof(char)];
        err = nvs_handle->get_string(key, buffer, required_size);
        if (err == ESP_OK) {
            // DO NOT log 'buffer' here because it may contain sensitive information.
            buffer[required_size] = '\0'; // Guarantee a null terminator.
        } else {
            delete[] buffer;
            buffer = nullptr;
        }
    }

    return buffer;
}


/**
 * The caller of this function is responsible for releasing the memory
 *  allocated and returned by this function.
 */
void *AppConfig::get_blob(const char *key)
{
    esp_err_t err;

    // Read the size of memory space required.
    size_t required_size = 0;  // value will default to 0, if not set yet in NVS
    err = nvs_handle->get_item_size(nvs::ItemType::BLOB_DATA, key, required_size);
    if (err != ESP_OK) {
        if (err == ESP_ERR_NVS_NOT_FOUND) {
            ESP_LOGW(LOG_TAG, "Warning (%s) - KEY NOT FOUND: '%s'",
                     esp_err_to_name(err), key);
        } else {
            ESP_LOGE(LOG_TAG, "Error (%s) - KEY: '%s'!",
                     esp_err_to_name(err), key);
        }
        return nullptr;
    }

    // Read previously saved value if available.
    char *buffer = nullptr;
    if (required_size > 0) {
        // The caller of this function is responsible for releasing the memory allocated to 'buffer'.
        // Add 1 to guarantee a null terminator.
        buffer = new char[required_size + sizeof(char)];
        err = nvs_handle->get_blob(key, buffer, required_size);
        if (err == ESP_OK) {
            // DO NOT log 'buffer' here because it may contain sensitive information.
            buffer[required_size] = '\0'; // Guarantee a null terminator.
        } else {
            delete[] buffer;
            buffer = nullptr;
        }
    }

    return buffer;
}


// const char *AppConfig::get_app_uuid()
// {
//     if (!app_uuid) {
//         app_uuid = get_str("app_uuid");
//         if (app_uuid) {
//             ESP_LOGI(LOG_TAG, "app_uuid: %s", app_uuid);
//         }
//     }
//     return app_uuid;
// }



//------------------------------------------------------------------------------
// WifiConfig
//------------------------------------------------------------------------------
const char *WifiConfig::get_wifi_ssid()
{
    if (!wifi_ssid) {
        wifi_ssid = get_str("wifi_ssid");
        if (wifi_ssid) {
            ESP_LOGI(LOG_TAG, "wifi_ssid: '%s'", wifi_ssid);
        }
    }
    return wifi_ssid;
}


const char *WifiConfig::get_wifi_password()
{
    if (!wifi_password) {
        wifi_password = get_str("wifi_password");
        if (wifi_password) {
            ESP_LOGI(LOG_TAG, "wifi_password: *redacted*");
        }
    }
    return wifi_password;
}
