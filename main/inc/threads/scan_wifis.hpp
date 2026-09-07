#pragma once

#include "esp_http_server.h"
#include "jaythread/thread_with_args.hpp"


class ScanWifisThread : public ThreadWithArg<httpd_req_t*> {
  public:
  void main(httpd_req_t** req);
};