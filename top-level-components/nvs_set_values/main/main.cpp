#include <stdio.h>
#include "freertos/FreeRTOS.h"
//#include "freertos/task.h"
//#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "credentials.h"

static const char *LOG_TAG = "app_main";


extern const uint8_t ca_cert_pem_start[] asm("_binary_mosq_ca_crt_start");
extern const uint8_t ca_cert_pem_end[] asm("_binary_mosq_ca_crt_end");
extern const uint8_t client_cert_pem_start[] asm("_binary_mosq_client_crt_start");
extern const uint8_t client_cert_pem_end[] asm("_binary_mosq_client_crt_end");
extern const uint8_t client_key_pem_start[] asm("_binary_mosq_client_key_start");
extern const uint8_t client_key_pem_end[] asm("_binary_mosq_client_key_end");

extern const uint8_t uuid4_txt_start[] asm("_binary_uuid4_txt_start");
extern const uint8_t uuid4_txt_end[] asm("_binary_uuid4_txt_end");



extern "C" void app_main(void)
{
    esp_err_t ret;

    ESP_LOGI(LOG_TAG, "[APP] Startup..");
    ESP_LOGI(LOG_TAG, "[APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
    ESP_LOGI(LOG_TAG, "[APP] IDF version: %s", esp_get_idf_version());

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("main", ESP_LOG_VERBOSE);

    //Initialize NVS
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    /****
    key,type,encoding,value

    global,namespace,,
    app_uuid,file,string,/project/private/uuid4.txt
    sntp_server,data,string,pool.ntp.org

    wifi,namespace,,
    ssid,file,string,/project/private/wifi_ssid.txt
    password,file,string,/project/private/wifi_password.txt

    mqtt,namespace,,
    broker_url,data,string,mqtts://raspberrypi:8883
    ca_cert,file,binary,/project/private/mosq_ca.crt
    client_cert,file,binary,/project/private/mosq_client.crt
    client_key,file,binary,/project/private/mosq_client.key
     ****/


    // (const char *)ca_cert_pem_start,
    // (const char *)client_cert_pem_start,
    // (const char *)client_key_pem_start,

    // (const char *)uuid4_txt_start,

}
