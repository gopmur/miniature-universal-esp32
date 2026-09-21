#pragma once

#include "http/module.hpp"
#include "jaythread/thread_with_args.hpp"
#include "system_logger.hpp"

class HttpCatThread : public ThreadWithArg<httpd_req_t*> {
  private:
  void main(httpd_req_t** req_p);
};

class HttpFsModule : public HttpModule {
  friend class HttpCatThread;

  MAKE_LOGGABLE("http_fs_module");

  private:
  static HttpCatThread http_cat_thread;
  static esp_err_t put_ls(httpd_req_t* req);
  static esp_err_t put_cat(httpd_req_t* req);
  void register_direct_uris();

  public:
  using HttpModule::HttpModule;
};