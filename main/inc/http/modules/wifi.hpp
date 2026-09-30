#pragma once

#include "http/module.hpp"
#include "jaythread/ipc/queue.hpp"
#include "jaythread/thread_with_args.hpp"

enum WifiConnectionRequestResult {
  OK,
  FAILED,
  WRONG_SSID,
  OTHER,
};

class HttpWifiModule : public HttpModule {
  MAKE_LOGGABLE("http_module");

  private:
  class GetScanAsyncHandler : public ThreadWithArg<httpd_req_t*> {
    private:
    void main(httpd_req_t** req);
  };

  class PutConnectionAsyncHandler : public ThreadWithArg<HttpJsonAsyncHandlerArgs> {
    MAKE_LOGGABLE("put_connect_async_handler");

    private:
    void main(HttpJsonAsyncHandlerArgs* args);

    public:
    Queue<WifiConnectionRequestResult, 1> connection_result_queue;
  };

  static GetScanAsyncHandler get_scan;
  static PutConnectionAsyncHandler put_connect;

  static esp_err_t get(httpd_req_t* req);
  static esp_err_t get_disconnect(httpd_req_t* req);

  void register_direct_uris();

  public:
  static Queue<WifiConnectionRequestResult, 1>* get_connection_result_queue();
  using HttpModule::HttpModule;
};