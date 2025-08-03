#pragma once

#include "esp_http_server.h"

class HTTPService {
 private:
  httpd_handle_t server_instance;

 public:
  void start();
};