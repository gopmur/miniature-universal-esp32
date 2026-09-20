#pragma once

#include "http/module.hpp"

class HttpSystemModule : public HttpModule {
  private:
  static esp_err_t get_restart(httpd_req_t* req);
  void register_direct_uris();

  public:
  using HttpModule::HttpModule;
};