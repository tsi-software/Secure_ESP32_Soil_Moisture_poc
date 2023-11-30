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

    // There is an NVS restriction on the maximum length of a string. ? 4000 bytes ?
    // Blobs have a much larger maximun size.
    // get_blob_as_str(...) allows us to circumvent the maximum string length restriction.
    // TODO: get the actual maximum string size and reference the URL.
    ConfigStr get_blob_as_str(const char *key);

    // Uncomment if needed...
    // ConfigBlob get_blob(const char *key);

private:
    std::unique_ptr<nvs::NVSHandle> nvs_handle;
    const char *nvs_namespace;
};



//------------------------------------------------------------------------------
class GlobalConfig: public AppConfig {
public:
    GlobalConfig() : AppConfig("global") { }
    const char *get_app_uuid();
    const char *get_sntp_server();

private:
    ConfigStr app_uuid;
    ConfigStr sntp_server;
};



//------------------------------------------------------------------------------
class WifiConfig: public AppConfig {
public:
    WifiConfig() : AppConfig("wifi") { }
    const char *get_ssid();
    const char *get_password();

private:
    ConfigStr ssid;
    ConfigStr password;
};



//------------------------------------------------------------------------------
class MqttConfig: public AppConfig {
public:
    MqttConfig() : AppConfig("mqtt") { }
    const char *get_broker_url();
    const char *get_ca_cert();
    const char *get_client_cert();
    const char *get_client_key();

private:
    ConfigStr broker_url;
    ConfigStr ca_cert;
    ConfigStr client_cert;
    ConfigStr client_key;
};



#endif // _CONFIG_HPP_
