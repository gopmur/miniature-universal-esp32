#pragma once

#include "esp_http_server.h"
#include "thread.hpp"

class WifiConnectionThread
    : public AbstractThread<WifiConnectionThread, httpd_req_t*> {
  public:
  WifiConnectionThread(const char* name, int priority, int stack_size);
  void main(httpd_req_t** req_p);
};
