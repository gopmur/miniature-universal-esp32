#pragma once

#include "http/module.hpp"
#include "jaythread/thread_with_args.hpp"
#include "system_logger.hpp"

class HttpSystemModule : public HttpModule {
  private:
  class GetCheckUpdateHandler : public ThreadWithArg<httpd_req_t*> {
    MAKE_LOGGABLE("get_check_update_handler");

    private:
    void main(httpd_req_t** rep_p);
  };

  static GetCheckUpdateHandler get_check_update;
  static esp_err_t get_version(httpd_req_t* req);
  static esp_err_t get_restart(httpd_req_t* req);
  static esp_err_t get_factory_reset(httpd_req_t* req);
  void register_direct_uris();

  public:
  using HttpModule::HttpModule;
};