#pragma once

#include "config.hpp"
#include "esp_http_server.h"
#include "ipc/queue.hpp"
#include "service.hpp"

enum class HttpAsyncHandlerMessageType { SCAN_WIFI };

struct HttpAsyncHandlerMessage {
  HttpAsyncHandlerMessageType type;
  httpd_req_t* req;
};

class HttpAsyncHandlerService
    : AbstractService<config::service::http_async_handler::stack_size> {
  private:
  static void main(HttpAsyncHandlerService* self);

  public:
  HttpAsyncHandlerService(int priority);
  void start();
  Queue<HttpAsyncHandlerMessage, 4> queue;
};