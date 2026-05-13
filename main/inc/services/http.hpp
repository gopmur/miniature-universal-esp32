#pragma once

#include "esp_err.h"
#include "esp_http_server.h"
#include "helper/json.hpp"
#include "ipc/queue.hpp"
#include "services/stm_uart/ssp.hpp"

enum class ControlMode : uint8_t {
  MANUAL,
  AUTO,
  SEMI_AUTO,
  SMART,
};

enum class HttpQueueMessageHeader {
  RUNNING,
  RIGHT_MANUAL_TORQUE,
  LEFT_MANUAL_TORQUE,
  MODE,
  LED_SERVICE_STACK_SIZE,
  IMU_SERVICE_STACK_SIZE,
  ESP_UART_TX_SERVICE_STACK_SIZE,
  ESP_UART_RX_SERVICE_STACK_SIZE,
  MOTOR_SERVICE_STACK_SIZE,
  CAN_RECV_SERVICE_STACK_SIZE,
  SD_SERVICE_STACK_SIZE,
  MONITOR_SERVICE_STACK_SIZE,
};

union HttpQueueMessagePayload {
  float f;
  uint32_t u32;
  bool b;
  ControlMode control_mode;
};

struct HttpQueueMessage {
  HttpQueueMessageHeader header;
  HttpQueueMessagePayload payload;
};

class HttpService {
  private:
  esp_err_t register_http_uri(const char* uri_address,
                              httpd_method_t method,
                              esp_err_t (*handler)(httpd_req_t* req));
  esp_err_t register_http_uri_with_option(const char* uri_address,
                                          httpd_method_t method,
                                          esp_err_t (*handler)(httpd_req_t* req));
  esp_err_t register_ws_uri(const char* uri_address, esp_err_t (*handler)(httpd_req_t* req));
  static void allow_cors(httpd_req_t* req);
  static void set_close_connection(httpd_req_t* req);
  static void set_type_json(httpd_req_t* req);
  static void set_header(httpd_req_t* req);

  static void set_rtc_time(JsonObject* time_json, JsonObject* time_error_json);
  static void set_rtc_date(JsonObject* date_json, JsonObject* date_error_json);

  static esp_err_t null_request_handler(httpd_req_t* req);
  
  static esp_err_t get_version_handler(httpd_req_t* req);

  static esp_err_t get_session_reports_handler(httpd_req_t* req);
  static esp_err_t start_handler(httpd_req_t* req);
  static esp_err_t stop_handler(httpd_req_t* req);
  static esp_err_t set_right_torque_handler(httpd_req_t* req);
  static esp_err_t set_left_torque_handler(httpd_req_t* req);
  static esp_err_t set_mode_manual_handler(httpd_req_t* req);
  static esp_err_t set_mode_automatic_handler(httpd_req_t* req);
  static esp_err_t set_mode_semi_automatic_handler(httpd_req_t* req);
  static esp_err_t set_mode_smart_handler(httpd_req_t* req);
  static esp_err_t set_rtc(httpd_req_t* req);
  static esp_err_t get_state_handler(httpd_req_t* req);
  static esp_err_t options_handler(httpd_req_t* req);
  static esp_err_t ws_data_handler(httpd_req_t* req);

  static esp_err_t start_stm_cpu_usage_stream_handler(httpd_req_t* req);
  static esp_err_t stop_stm_cpu_usage_stream_handler(httpd_req_t* req);
  static esp_err_t start_esp_cpu_usage_stream_handler(httpd_req_t* req);
  static esp_err_t stop_esp_cpu_usage_stream_handler(httpd_req_t* req);
  static esp_err_t start_imu_data_stream_handler(httpd_req_t* req);
  static esp_err_t stop_imu_data_stream_handler(httpd_req_t* req);
  static esp_err_t start_motor_data_stream_handler(httpd_req_t* req);
  static esp_err_t stop_motor_data_stream_handler(httpd_req_t* req);

  static esp_err_t get_stm_task_stack_size(httpd_req_t* req);
  static esp_err_t get_esp_task_stack_size(httpd_req_t* req);

  static esp_err_t scan_wifi_handler(httpd_req_t* req);
  static esp_err_t connect_to_wifi_handler(httpd_req_t* req);
  static esp_err_t disconnect_wifi_handler(httpd_req_t* req);
  static esp_err_t get_connected_wifi(httpd_req_t* req);

  static esp_err_t restart_handler(httpd_req_t* req);
  static esp_err_t restart_stm32_handler(httpd_req_t* req);
  static esp_err_t restart_esp32_handler(httpd_req_t* req);

  static esp_err_t check_for_update_handler(httpd_req_t* req);
  static esp_err_t update_handler(httpd_req_t* req);
  static esp_err_t get_ota_status(httpd_req_t* req);

  esp_err_t register_dynamic_endpoints();

  public:
  static constexpr const char* LOG_TAG = "HTTP Service";
  httpd_handle_t server_instance;
  Queue<HttpQueueMessage, 8> state_queue;
  Queue<HttpQueueMessage, 16> task_stack_size_queue;
  void start();
};