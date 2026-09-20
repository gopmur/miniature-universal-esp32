#pragma once

#include "esp_http_server.h"
#include "jaythread/ipc/queue.hpp"
#include "jaythread/thread.hpp"
#include "loggable.hpp"

enum WifiConnectionRequestResult {
  OK,
  FAILED,
  WRONG_SSID,
  OTHER,
};

class WifiConHandlerTask : public Thread {
  MAKE_LOGGABLE("wifi_con_handler_task");

  private:
  void main();

  public:
  Queue<httpd_req_t*, 1> req_queue;
  Queue<WifiConnectionRequestResult, 1> connection_result_queue;
};
