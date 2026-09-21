#pragma once

#include "http/module.hpp"
#include "system_logger.hpp"

class HttpFsModule : public HttpModule {
  MAKE_LOGGABLE("http_fs_module");

  private:
  static esp_err_t put_ls(httpd_req_t* req);
  void register_direct_uris();

  public:
  using HttpModule::HttpModule;
};