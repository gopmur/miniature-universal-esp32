#pragma once

#include "esp_err.h"
#include "esp_http_server.h"
#include "ipc/queue.hpp"
#include "services/stm_uart/packet.hpp"

enum class HttpQueueMessageHeader {
  RUNNING,
  RIGHT_MANUAL_TORQUE,
  LEFT_MANUAL_TORQUE,
  MODE,
};

union HttpQueueMessagePayload {
  float f;
  bool b;
  ControlMode control_mode;
};

struct HttpQueueMessage {
  HttpQueueMessageHeader header;
  HttpQueueMessagePayload payload;
};

class HttpService {
  private:
  httpd_handle_t server_instance;

  static constexpr const char* LOG_TAG = "HTTP Service";
  static esp_err_t get_session_reports_handler(httpd_req_t* req);
  static esp_err_t start_handler(httpd_req_t* req);
  static esp_err_t stop_handler(httpd_req_t* req);
  static esp_err_t set_right_torque_handler(httpd_req_t* req);
  static esp_err_t set_left_torque_handler(httpd_req_t* req);
  static esp_err_t set_mode_manual_handler(httpd_req_t* req);
  static esp_err_t set_mode_automatic_handler(httpd_req_t* req);
  static esp_err_t set_mode_semi_automatic_handler(httpd_req_t* req);
  static esp_err_t set_mode_smart_handler(httpd_req_t* req);
  static esp_err_t get_state_handler(httpd_req_t* req);

  esp_err_t register_dynamic_endpoints();

  public:
  Queue<HttpQueueMessage, 8> queue;
  void start();
};