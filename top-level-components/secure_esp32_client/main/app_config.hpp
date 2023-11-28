/*
app_config.hpp
*/
#ifndef _CONFIG_HPP_
#define _CONFIG_HPP_

#include <memory>
#include "nvs_handle.hpp"


//------------------------------------------------------------------------------
class AppConfig {
public:
    virtual ~AppConfig() {
    }

protected:
    AppConfig(const char *nvs_namespace);

    // The caller of this function is responsible for releasing the memory
    //  allocated and returned by this function.
    char *get_str(const char *key);

    // The caller of this function is responsible for releasing the memory
    //  allocated and returned by this function.
    void *get_blob(const char *key);

private:
    std::unique_ptr<nvs::NVSHandle> nvs_handle;
    const char *nvs_namespace;
};



// class GlobalConfig: public AppConfig {
    //const char *get_app_uuid();
//    char *app_uuid = nullptr;
// }
        //delete[] app_uuid;


//------------------------------------------------------------------------------
class WifiConfig: public AppConfig {
public:
    WifiConfig() : AppConfig("wifi") { }
    virtual ~WifiConfig() {
        delete[] wifi_ssid;
        delete[] wifi_password;
    }
    const char *get_wifi_ssid();
    const char *get_wifi_password();

private:
    char *wifi_ssid = nullptr;
    char *wifi_password = nullptr;
};


#endif // _CONFIG_HPP_
