#pragma once

#include "http/module.hpp"
#include "jaythread/thread_with_args.hpp"
#include "system_logger.hpp"

class HttpFsModule : public HttpModule {
  MAKE_LOGGABLE("http_fs_module");

  private:
  class PutCatAsyncHandler : public ThreadWithArg<HttpJsonAsyncHandlerArgs> {
    private:
    void main(HttpJsonAsyncHandlerArgs* args_p);
  };

  static PutCatAsyncHandler put_cat;
  static esp_err_t put_ls(httpd_req_t* req, JsonObject* req_json);
  void register_direct_uris();

  public:
  using HttpModule::HttpModule;
};