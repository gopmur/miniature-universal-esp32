#pragma once

#include "esp_err.h"
#include "esp_http_server.h"
#include "helper/json.hpp"
#include "jaythread/ipc/queue.hpp"


class HttpService {
  private:
  esp_err_t register_http_uri(const char* uri_address,
                              httpd_method_t method,
                              esp_err_t (*handler)(httpd_req_t* req));
  esp_err_t register_http_uri_with_option(const char* uri_address,
                                          httpd_method_t method,
                                          esp_err_t (*handler)(httpd_req_t* req));
  esp_err_t register_ws_uri(const char* uri_address,
                            esp_err_t (*handler)(httpd_req_t* req),
                            esp_err_t (*post_handshake_handler)(httpd_req_t* req));
  static void allow_cors(httpd_req_t* req);
  static void set_close_connection(httpd_req_t* req);
  static void set_type_json(httpd_req_t* req);
  static void set_header(httpd_req_t* req);

  static void set_rtc_time(JsonObject* time_json, JsonObject* time_error_json);
  static void set_rtc_date(JsonObject* date_json, JsonObject* date_error_json);

  static esp_err_t send_json(httpd_req_t* req, JsonObject& json);
  static esp_err_t send_json(httpd_req_t* req, JsonObject& json, httpd_err_code_t status);

  static esp_err_t null_request_handler(httpd_req_t* req);

  static esp_err_t get_version_handler(httpd_req_t* req);

  static esp_err_t get_session_reports_handler(httpd_req_t* req);
  static esp_err_t start_handler(httpd_req_t* req);
  static esp_err_t stop_handler(httpd_req_t* req);
  static esp_err_t set_mode_manual_handler(httpd_req_t* req);
  static esp_err_t set_mode_automatic_handler(httpd_req_t* req);
  static esp_err_t set_mode_semi_automatic_handler(httpd_req_t* req);
  static esp_err_t set_mode_smart_handler(httpd_req_t* req);
  static esp_err_t set_rtc(httpd_req_t* req);
  static esp_err_t get_state_handler(httpd_req_t* req);
  static esp_err_t options_handler(httpd_req_t* req);
  static esp_err_t ws_data_handler(httpd_req_t* req);
  static esp_err_t ws_imu_stream_handler(httpd_req_t* req);
  static esp_err_t ws_imu_stream_post_handshake_handler(httpd_req_t* req);
  static esp_err_t ws_data_post_handshake_handler(httpd_req_t* req);
  static esp_err_t set_automatic_control_params(httpd_req_t* req);
  static esp_err_t set_semiautomatic_control_params(httpd_req_t* req);
  static esp_err_t set_smart_control_params(httpd_req_t* req);

  static esp_err_t start_cpu_usage_stream_handler(httpd_req_t* req);
  static esp_err_t stop_cpu_usage_stream_handler(httpd_req_t* req);
  static esp_err_t start_imu_data_stream_handler(httpd_req_t* req);
  static esp_err_t stop_imu_data_stream_handler(httpd_req_t* req);
  static esp_err_t start_motor_data_stream_handler(httpd_req_t* req);
  static esp_err_t stop_motor_data_stream_handler(httpd_req_t* req);

  static esp_err_t scan_wifi_handler(httpd_req_t* req);
  static esp_err_t connect_to_wifi_handler(httpd_req_t* req);
  static esp_err_t disconnect_wifi_handler(httpd_req_t* req);
  static esp_err_t get_connected_wifi(httpd_req_t* req);

  static esp_err_t restart_handler(httpd_req_t* req);

  static esp_err_t check_for_update_handler(httpd_req_t* req);
  static esp_err_t update_handler(httpd_req_t* req);
  static esp_err_t get_ota_status(httpd_req_t* req);

  static esp_err_t motor_zero_pos(httpd_req_t* req);

  static esp_err_t start_log(httpd_req_t* req);
  static esp_err_t stop_log(httpd_req_t* req);

  esp_err_t register_dynamic_endpoints();

  public:
  static constexpr const char* LOG_TAG = "HTTP Service";
  httpd_handle_t server_instance;
  void start();
};