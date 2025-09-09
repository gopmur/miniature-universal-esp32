#pragma once

#include "esp_err.h"
#include "esp_http_server.h"

class HTTPService {
 private:
  httpd_handle_t server_instance;

  static constexpr const char* LOG_TAG = "HTTP Service";
  static esp_err_t get_session_reports_handler(httpd_req_t* req);

 public:
  void start();
};