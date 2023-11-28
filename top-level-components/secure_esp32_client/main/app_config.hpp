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
    virtual ~AppConfig() { }

protected:
    using ConfigStr_T  = char;
    using ConfigBlob_T = std::byte;
    using ConfigStr  = std::unique_ptr< ConfigStr_T[] >;
    using ConfigBlob = std::unique_ptr< ConfigBlob_T[] >;

    AppConfig(const char *nvs_namespace);
    ConfigStr get_str(const char *key);
    ConfigBlob get_blob(const char *key);

private:
    std::unique_ptr<nvs::NVSHandle> nvs_handle;
    const char *nvs_namespace;
};



//------------------------------------------------------------------------------
class GlobalConfig: public AppConfig {
public:
    GlobalConfig() : AppConfig("global") { }
    const char *get_app_uuid();

private:
    ConfigStr app_uuid;
};



//------------------------------------------------------------------------------
class WifiConfig: public AppConfig {
public:
    WifiConfig() : AppConfig("wifi") { }
    const char *get_wifi_ssid();
    const char *get_wifi_password();

private:
    ConfigStr wifi_ssid;
    ConfigStr wifi_password;
};


#endif // _CONFIG_HPP_
