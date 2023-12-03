/*
app_config.cpp
*/

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
        ESP_LOGW(LOG_TAG, "Warning (%s) opening NVS handle for namespace '%s'!",
                 esp_err_to_name(err), nvs_namespace);
    }
}



AppConfig::ConfigStr AppConfig::get_str(const char *key)
{
    esp_err_t err;
    ConfigStr null_result; // Default return value is an null unique_ptr.

    if (!nvs_handle) {
        ESP_LOGW(LOG_TAG, "Warning - get_str(%s) - NVS handle not opened for namespace '%s'!", key, nvs_namespace);
        return null_result;
    }

    // Read the size of memory space required.
    size_t required_size = 0;  // size value will default to 0 if not set yet in NVS.
    err = nvs_handle->get_item_size(nvs::ItemType::SZ, key, required_size);
    if (err != ESP_OK) {
        if (err == ESP_ERR_NVS_NOT_FOUND) {
            ESP_LOGW(LOG_TAG, "Warning (%s) - KEY NOT FOUND: '%s':'%s'",
                     esp_err_to_name(err), nvs_namespace, key);
        } else {
            ESP_LOGE(LOG_TAG, "Error (%s) - KEY: '%s':'%s'!",
                     esp_err_to_name(err), nvs_namespace, key);
        }
        return null_result;
    }

    // Read the previously saved value if available.
    ConfigStr buffer;
    if (required_size > 0) {
        // Add 1 to guarantee a null terminator.
        buffer = ConfigStr( new ConfigStr_T[required_size + 1] );
        err = nvs_handle->get_string(key, buffer.get(), required_size);
        if (err == ESP_OK) {
            // DO NOT log 'buffer' here because it may contain sensitive information.
            buffer[required_size] = '\0'; // Guarantee a null terminator.
        } else {
            return null_result;
        }
    }

    return buffer;
}



/**
  There is an NVS restriction on the maximum length of a string. ? 4000 bytes ?
  Blobs have a much larger maximun size.
  get_blob_as_str(...) allows us to circumvent the maximum string length restriction.

  TODO: get the actual maximum string size and reference the URL.
*/
AppConfig::ConfigStr AppConfig::get_blob_as_str(const char *key)
{
    esp_err_t err;
    ConfigStr null_result; // Default return value is an null unique_ptr.

    if (!nvs_handle) {
        ESP_LOGW(LOG_TAG, "Warning - get_blob_as_str(%s) - NVS handle not opened for namespace '%s'!",
                 key, nvs_namespace);
        return null_result;
    }

    // Read the size of memory space required.
    size_t required_size = 0;  // value will default to 0, if not set yet in NVS
    err = nvs_handle->get_item_size(nvs::ItemType::BLOB, key, required_size);
    if (err != ESP_OK) {
        if (err == ESP_ERR_NVS_NOT_FOUND) {
            ESP_LOGW(LOG_TAG, "Warning (%s) - KEY NOT FOUND: '%s':'%s'",
                     esp_err_to_name(err), nvs_namespace, key);
        } else {
            ESP_LOGE(LOG_TAG, "Error (%s) - KEY: '%s':'%s'!",
                     esp_err_to_name(err), nvs_namespace, key);
        }
        return null_result;
    }

    // Read the previously saved value if available.
    ConfigStr buffer;
    if (required_size > 0) {
        // Add 1 to guarantee a null terminator.
        buffer = ConfigStr( new ConfigStr_T[required_size + 1] );
        err = nvs_handle->get_blob(key, buffer.get(), required_size);
        if (err == ESP_OK) {
            // DO NOT log 'buffer' here because it may contain sensitive information.
            buffer[required_size] = ConfigStr_T(0); // Guarantee a null terminator.
        } else {
            return null_result;
        }
    }

    return buffer;
}


/**
// Uncomment if needed...
// TODO: data length must also be returned!

AppConfig::ConfigBlob AppConfig::get_blob(const char *key)
{
    esp_err_t err;
    ConfigBlob null_result; // Default return value is an null unique_ptr.

    if (!nvs_handle) {
        ESP_LOGW(LOG_TAG, "Warning - get_blob(%s) - NVS handle not opened for namespace '%s'!", key, nvs_namespace);
        return null_result;
    }

    // Read the size of memory space required.
    size_t required_size = 0;  // value will default to 0, if not set yet in NVS
    err = nvs_handle->get_item_size(nvs::ItemType::BLOB, key, required_size);
    if (err != ESP_OK) {
        if (err == ESP_ERR_NVS_NOT_FOUND) {
            ESP_LOGW(LOG_TAG, "Warning (%s) - KEY NOT FOUND: '%s':'%s'",
                     esp_err_to_name(err), nvs_namespace, key);
        } else {
            ESP_LOGE(LOG_TAG, "Error (%s) - KEY: '%s':'%s'!",
                     esp_err_to_name(err), nvs_namespace, key);
        }
        return null_result;
    }

    // Read the previously saved value if available.
    ConfigBlob buffer;
    if (required_size > 0) {
        //TODO: remove the concept of "null terminator" for this BLOB!
        // Add 1 to guarantee a null terminator.
        buffer = ConfigBlob( new ConfigBlob_T[required_size + 1] );
        err = nvs_handle->get_blob(key, buffer.get(), required_size);
        if (err == ESP_OK) {
            // DO NOT log 'buffer' here because it may contain sensitive information.
            buffer[required_size] = ConfigBlob_T(0); // Guarantee a null terminator.
        } else {
            return null_result;
        }
    }

    return buffer;
}
**/
