#pragma once

#include "esp_err.h"
#include "esp_http_server.h"
#include "http/module.hpp"
#include "jaythread/thread_with_args.hpp"
#include "system_logger.hpp"

class HttpUpdateModule : public HttpModule {
  MAKE_LOGGABLE("http_update_module");

  private:
  class GetCheckHandler : public ThreadWithArg<httpd_req_t*> {
    MAKE_LOGGABLE(HttpUpdateModule::tag);
    private:
    void main(httpd_req_t** req_p);
  };

  class PutHandler : public ThreadWithArg<HttpJsonAsyncHandlerArgs> {
    MAKE_LOGGABLE(HttpUpdateModule::tag);
    private:
    void send_failed_message(httpd_req_t* req, esp_err_t err);
    void main(HttpJsonAsyncHandlerArgs* req_p);
  };

  static bool ota_busy;

  static GetCheckHandler get_check;
  static PutHandler put;
  static esp_err_t get_ota_status(httpd_req_t* req);

  void register_direct_uris();

  public:
  using HttpModule::HttpModule;
};