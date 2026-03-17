#pragma once

#include "esp_http_server.h"
#include "thread.hpp"

class ScanWifisThread : public AbstractThread<ScanWifisThread, httpd_req_t*> {
  public:
  ScanWifisThread(const char* name, int priority, int stack_size);
  void main(httpd_req_t** req);
};