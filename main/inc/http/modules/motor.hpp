#pragma once

#include "http/module.hpp"

class HttpMotorModule : public HttpModule {
  MAKE_LOGGABLE("http_motor_module");

  private:
  static esp_err_t get_zero_pos(httpd_req_t* req);
  void register_direct_uris();

  public:
  using HttpModule::HttpModule;
};