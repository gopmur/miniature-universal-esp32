#pragma once

#include "esp_http_server.h"
#include "http/module.hpp"
#include "system_logger.hpp"

class HttpLegacyModule : public HttpModule {
  MAKE_LOGGABLE("http_legacy_module");

  private:
  static void send_resp(httpd_req_t* req, JsonObject json);
  static esp_err_t handle_ping(httpd_req_t* req);
  static esp_err_t handle_enable(httpd_req_t* req);
  static esp_err_t handle_disable(httpd_req_t* req);

  static esp_err_t ws(httpd_req_t* req);
  static esp_err_t ws_post_handshake(httpd_req_t* req);
  void register_direct_uris();

  public:
  using HttpModule::HttpModule;
};