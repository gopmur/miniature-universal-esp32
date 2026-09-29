#pragma once

#include "http/module.hpp"
#include "jaythread/thread_with_args.hpp"
#include "system_logger.hpp"

struct HttpCatThreadArgs {
  httpd_req_t* req;
  JsonObject json;
};

class HttpCatThread : public ThreadWithArg<HttpCatThreadArgs> {
  private:
  void main(HttpCatThreadArgs* args_p);
};

class HttpFsModule : public HttpModule {
  friend class HttpCatThread;

  MAKE_LOGGABLE("http_fs_module");

  private:
  static HttpCatThread http_cat_thread;
  static esp_err_t put_ls(httpd_req_t* req, JsonObject* req_json);
  static esp_err_t put_cat(httpd_req_t* req, JsonObject* req_json);
  void register_direct_uris();

  public:
  using HttpModule::HttpModule;
};