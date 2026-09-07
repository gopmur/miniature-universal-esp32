#pragma once

#include "esp_http_server.h"
#include "ipc/queue.hpp"
#include "jaythread/thread.hpp"

enum WifiConnectionRequestResult {
  OK,
  FAILED,
  WRONG_SSID,
  OTHER,
};

class HttpWifiConHandlerService : public Thread {
  private:
  void main();

  public:
  Queue<httpd_req_t*, 1> req_queue;
  Queue<WifiConnectionRequestResult, 1> connection_result_queue;
};
