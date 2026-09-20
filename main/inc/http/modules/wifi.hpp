#pragma once

#include "helper/ds/static_string.hpp"
#include "http/module.hpp"
#include "http/modules/wifi/scan_wifis.hpp"

class HttpWifiModule : public HttpModule {
  MAKE_LOGGABLE("http_module");

  private:
  static ScanWifisThread scan_wifis_thread;

  static esp_err_t get_scan(httpd_req_t* req);
  static esp_err_t get(httpd_req_t* req);
  static esp_err_t put_connect(httpd_req_t* req);
  static esp_err_t get_disconnect(httpd_req_t* req);
  
  void register_direct_uris();

  public:
  using HttpModule::HttpModule;
};