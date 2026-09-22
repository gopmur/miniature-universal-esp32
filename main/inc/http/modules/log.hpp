#pragma once

#include "http/module.hpp"

class HttpLogModule : public HttpModule {
  MAKE_LOGGABLE("http_log_module");

  private:
  static esp_err_t get_start(httpd_req_t* req);
  static esp_err_t get_stop(httpd_req_t* req);
  static esp_err_t ws_system(httpd_req_t* req);
  static esp_err_t ws_system_post_handshake(httpd_req_t* req);
  void register_direct_uris();

  public:
  using HttpModule::HttpModule;
};