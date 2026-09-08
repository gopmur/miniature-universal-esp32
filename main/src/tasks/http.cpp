#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <format>
#include <variant>

#include "tasks/http.hpp"

#include "config.hpp"
#include "driver/uart.h"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_http_client.h"
#include "esp_http_server.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/idf_additions.h"
#include "helper.hpp"
#include "helper/json.hpp"
#include "http_assets.hpp"
#include "http_parser.h"


#include "jaythread/sync.hpp"
#include "tasks/http/helper.hpp"
#include "tasks/http_wifi_con_handler.hpp"
#include "tasks/ws.hpp"
#include "threads/get_stack_sizes.hpp"
#include "threads/get_states.hpp"
#include "threads/scan_wifis.hpp"
#include "version.hpp"

esp_err_t HttpService::send_json(httpd_req_t* req, JsonObject& json) {
  auto json_string = json.stringify();
  httpd_resp_send(req, json_string.c_str(), HTTPD_RESP_USE_STRLEN);
  return ESP_OK;
}

esp_err_t HttpService::send_json(httpd_req_t* req, JsonObject& json, httpd_err_code_t status) {
  auto json_string = json.stringify();
  httpd_resp_send_err(req, status, json_string.c_str());
  return ESP_OK;
}

esp_err_t HttpService::null_request_handler(httpd_req_t* req) {
  set_header(req);
  httpd_resp_send(req, nullptr, 0);
  return ESP_OK;
}

extern WebSocketService ws_service;
ScanWifisThread scan_wifis_thread;

esp_err_t HttpService::scan_wifi_handler(httpd_req_t* req) {
  set_header(req);
  httpd_req_t* async_req;
  httpd_req_async_handler_begin(req, &async_req);
  scan_wifis_thread.start("ws_service", 2, 4096, async_req);
  return ESP_OK;
}

esp_err_t HttpService::get_connected_wifi(httpd_req_t* req) {
  set_header(req);
  wifi_ap_record_t ap_info;
  auto result = esp_wifi_sta_get_ap_info(&ap_info);
  JsonObject res_json;
  if (result == ESP_ERR_WIFI_NOT_CONNECT) {
    res_json.set("connected", false);
    res_json.set("errorMessage", "not connected");
  } else if (result == ESP_ERR_WIFI_CONN) {
    res_json.set("connected", false);
    res_json.set("errorMessage", "wifi not initialized");
  } else {
    res_json.set("connected", true);
    res_json.set("ssid", reinterpret_cast<char*>(ap_info.ssid));
    res_json.set("rssi", ap_info.rssi);
    auto bssid_str = get_bssid_string(ap_info.bssid);
    res_json.set("bssid", bssid_str.get_data());
  };
  auto res_str = res_json.stringify();
  httpd_resp_send(req, res_str.c_str(), HTTPD_RESP_USE_STRLEN);
  return ESP_OK;
}

extern HttpWifiConHandlerService http_wifi_con_handler_service;

esp_err_t HttpService::connect_to_wifi_handler(httpd_req_t* req) {
  set_header(req);
  httpd_req_t* async_req;
  httpd_req_async_handler_begin(req, &async_req);
  auto service_not_busy = http_wifi_con_handler_service.req_queue.send(async_req, 0);
  if (!service_not_busy) {
    JsonObject res_json;
    res_json.set("message", "another connection request is pending");
    auto res_str = res_json.stringify();
    httpd_resp_send_err(async_req, HTTPD_500_INTERNAL_SERVER_ERROR, res_str.c_str());
  }
  return ESP_OK;
}

esp_err_t HttpService::disconnect_wifi_handler(httpd_req_t* req) {
  set_header(req);
  esp_wifi_disconnect();
  httpd_resp_send(req, nullptr, 0);
  return ESP_OK;
}

// void HttpService::set_rtc_time(JsonObject* time_json, JsonObject* time_error_json) {
//   auto hours_item = time_json->get_number("hours", time_error_json);
//   auto minutes_item = time_json->get_number("minutes", time_error_json);
//   auto seconds_item = time_json->get_number("seconds", time_error_json);
//   if (std::holds_alternative<JsonError>(hours_item) ||
//       std::holds_alternative<JsonError>(minutes_item) ||
//       std::holds_alternative<JsonError>(seconds_item)) {
//     return;
//   }
//   uint8_t hours = std::get<double>(hours_item);
//   uint8_t minutes = std::get<double>(minutes_item);
//   uint8_t seconds = std::get<double>(seconds_item);
//   std::array<uint8_t, 4> time_data = {hours, minutes, seconds, 0};
//   write_address(SspAddress::RTC_TIME, time_data);
// }

// void HttpService::set_rtc_date(JsonObject* date_json, JsonObject* date_error_json) {
//   auto year_item = date_json->get_number("year", date_error_json);
//   auto month_item = date_json->get_number("month", date_error_json);
//   auto day_item = date_json->get_number("day", date_error_json);
//   if (std::holds_alternative<JsonError>(year_item) ||
//       std::holds_alternative<JsonError>(month_item) ||
//       std::holds_alternative<JsonError>(day_item)) {
//     return;
//   }
//   uint16_t year = std::get<double>(year_item);
//   uint8_t month = std::get<double>(month_item);
//   uint8_t day = std::get<double>(day_item);
//   std::array<uint8_t, 4> date_data = {get_byte(year, 1), get_byte(year, 0), month, day};
//   write_address(SspAddress::RTC_DATE, date_data);
// }

// esp_err_t HttpService::set_rtc(httpd_req_t* req) {
//   set_header(req);
//   char* req_body = static_cast<char*>(malloc(req->content_len + 1));
//   int received = httpd_req_recv(req, req_body, req->content_len);
//   req_body[received] = 0;

//   JsonObject res_json;
//   auto req_json_result = JsonObject::parse(req_body);
//   free(req_body);

//   if (std::holds_alternative<JsonError>(req_json_result)) {
//     res_json.set("message", "parse error");
//     auto res_str = res_json.stringify();
//     httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, res_str.c_str());
//     return ESP_OK;
//   }

//   auto req_json = std::get<JsonObject>(req_json_result);

//   auto time_item = req_json.get_object("time", &res_json);
//   auto date_item = req_json.get_object("date", &res_json);

//   if (std::holds_alternative<JsonObject>(time_item)) {
//     JsonObject time_json = std::get<JsonObject>(time_item);
//     JsonObject time_error_json;
//     set_rtc_time(&time_json, &time_error_json);
//     if (!time_error_json.is_empty()) {
//       res_json.set("time", &time_error_json);
//     }
//   }

//   if (std::holds_alternative<JsonObject>(date_item)) {
//     JsonObject date_json = std::get<JsonObject>(date_item);
//     JsonObject date_error_json;
//     set_rtc_date(&date_json, &date_error_json);
//     if (!date_error_json.is_empty()) {
//       res_json.set("date", &date_error_json);
//     }
//   }

//   auto res_str = res_json.stringify();
//   if (res_json.is_empty()) {
//     httpd_resp_send(req, res_str.c_str(), HTTPD_RESP_USE_STRLEN);
//   } else {
//     httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, res_str.c_str());
//   }

//   return ESP_OK;
// }

void HttpService::allow_cors(httpd_req_t* req) {
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "GET, PUT, POST, OPTIONS");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "Content-Type");
}

void HttpService::set_close_connection(httpd_req_t* req) {
  httpd_resp_set_hdr(req, "Connection", "close");
}
void HttpService::set_type_json(httpd_req_t* req) {
  httpd_resp_set_type(req, "application/json");
}
void HttpService::set_header(httpd_req_t* req) {
  HttpService::allow_cors(req);
  HttpService::set_close_connection(req);
  HttpService::set_type_json(req);
}

// esp_err_t HttpService::get_esp_task_stack_size(httpd_req_t* req) {
//   set_header(req);
//   JsonObject res_json;

//   for (auto service : Thread::get_thread_list()) {
//     auto service_name = service.get_name();
//     if (service.stack_size) {
//       res_json.set(service_name, service.stack_size);
//     }
//   }

//   auto res_str = res_json.stringify();

//   httpd_resp_send(req, res_str.c_str(), HTTPD_RESP_USE_STRLEN);

//   return ESP_OK;
// }

// esp_err_t HttpService::get_stm_task_stack_size(httpd_req_t* req) {
//   set_header(req);
//   http_service.task_stack_size_queue.flush();

//   read_addresses({SspAddress::LED_SERVICE_STACK_SIZE,
//                   SspAddress::IMU_SERVICE_STACK_SIZE,
//                   SspAddress::ESP_UART_TX_SERVICE_STACK_SIZE,
//                   SspAddress::ESP_UART_RX_SERVICE_STACK_SIZE,
//                   SspAddress::MOTOR_SERVICE_STACK_SIZE,
//                   SspAddress::CAN_RECV_SERVICE_STACK_SIZE,
//                   SspAddress::SD_SERVICE_STACK_SIZE,
//                   SspAddress::MONITOR_SERVICE_STACK_SIZE});

//   GetStackSizes async_handler("get_stack_sized", 5, 4096);
//   httpd_req_t* async_req;
//   httpd_req_async_handler_begin(req, &async_req);
//   async_handler.start(&async_req);
//   return ESP_OK;
// }
const char* get_control_mode_str(ControlMode control_mode) {
  switch (control_mode) {
    case ControlMode::MANUAL:
      return "manual";
    case ControlMode::AUTO:
      return "automatic";
    case ControlMode::SEMI_AUTO:
      return "semi-automatic";
    case ControlMode::SMART:
      return "smart";
  }
  return "undefined";
}

const char* get_leg_str(Leg leg) {
  switch (leg) {
    case Leg::LEFT:
      return "left";
    case Leg::RIGHT:
      return "right";
  }
  return "undefined";
}

esp_err_t HttpService::get_state_handler(httpd_req_t* req) {
  set_header(req);
  JsonObject res_json;
  JsonObject control_params_json;
  JsonObject manual_control_params_json;
  JsonObject automatic_control_params_json;
  JsonObject semiautomatic_control_params_json;
  JsonObject smart_control_params_json;
  JsonObject manual_control_params_left_json;
  JsonObject automatic_control_params_left_json;
  JsonObject semiautomatic_control_params_left_json;
  JsonObject smart_control_params_left_json;
  JsonObject manual_control_params_right_json;
  JsonObject automatic_control_params_right_json;
  JsonObject semiautomatic_control_params_right_json;
  JsonObject smart_control_params_right_json;

  res_json.set("running", control_state.running);
  res_json.set("mode", get_control_mode_str(control_state.control_mode));
  manual_control_params_left_json.set("torque", control_state.control_params.manual.left.torque);
  manual_control_params_right_json.set("torque", control_state.control_params.manual.right.torque);
  automatic_control_params_left_json.set("torque",
                                         control_state.control_params.automatic.left.torque);
  automatic_control_params_left_json.set("timeout",
                                         control_state.control_params.automatic.left.timeout);
  automatic_control_params_left_json.set(
      "velocityThreshold",
      control_state.control_params.automatic.left.velocity_threshold);
  automatic_control_params_right_json.set("torque",
                                          control_state.control_params.automatic.right.torque);
  automatic_control_params_right_json.set("timeout",
                                          control_state.control_params.automatic.right.timeout);
  automatic_control_params_right_json.set(
      "velocityThreshold",
      control_state.control_params.automatic.right.velocity_threshold);
  semiautomatic_control_params_json.set(
      "weakLeg",
      get_leg_str(control_state.control_params.semiautomatic.weak_leg));
  semiautomatic_control_params_left_json.set(
      "torque",
      control_state.control_params.semiautomatic.left.torque);
  semiautomatic_control_params_left_json.set(
      "timeout",
      control_state.control_params.semiautomatic.left.timeout);
  semiautomatic_control_params_left_json.set("delay",
                                             control_state.control_params.semiautomatic.left.delay);
  semiautomatic_control_params_right_json.set(
      "torque",
      control_state.control_params.semiautomatic.right.torque);
  semiautomatic_control_params_right_json.set(
      "timeout",
      control_state.control_params.semiautomatic.right.timeout);
  semiautomatic_control_params_right_json.set(
      "delay",
      control_state.control_params.semiautomatic.right.delay);
  semiautomatic_control_params_json.set(
      "startAssistAngle",
      control_state.control_params.semiautomatic.start_assist_angle);
  semiautomatic_control_params_json.set(
      "stopAssistAngle",
      control_state.control_params.semiautomatic.stop_assist_angle);
  smart_control_params_left_json.set("torque", control_state.control_params.smart.left.torque);
  smart_control_params_right_json.set("torque", control_state.control_params.smart.right.torque);

  manual_control_params_json.set("right", &manual_control_params_right_json);
  manual_control_params_json.set("left", &manual_control_params_left_json);
  automatic_control_params_json.set("right", &automatic_control_params_right_json);
  automatic_control_params_json.set("left", &automatic_control_params_left_json);
  semiautomatic_control_params_json.set("right", &semiautomatic_control_params_right_json);
  semiautomatic_control_params_json.set("left", &semiautomatic_control_params_left_json);
  smart_control_params_json.set("right", &smart_control_params_right_json);
  smart_control_params_json.set("left", &smart_control_params_left_json);
  control_params_json.set("manual", &manual_control_params_json);
  control_params_json.set("automatic", &automatic_control_params_json);
  control_params_json.set("semiautomatic", &semiautomatic_control_params_json);
  control_params_json.set("smart", &smart_control_params_json);
  res_json.set("controlParams", &control_params_json);

  auto json_str = res_json.stringify();
  auto ret = httpd_resp_send(req, json_str.c_str(), HTTPD_RESP_USE_STRLEN);
  if (ret != ESP_OK) {
    ESP_LOGE(HttpService::LOG_TAG, "Get running response transmission failed");
  }
  return ESP_OK;
}

esp_err_t HttpService::start_handler(httpd_req_t* req) {
  set_header(req);
  control_state.running = true;
  ESP_RETURN_ON_ERROR(httpd_resp_send(req, nullptr, 0),
                      HttpService::LOG_TAG,
                      "Start response transmission failed");
  return ESP_OK;
}

esp_err_t HttpService::stop_handler(httpd_req_t* req) {
  set_header(req);
  control_state.running = false;
  ESP_RETURN_ON_ERROR(httpd_resp_send(req, nullptr, 0),
                      HttpService::LOG_TAG,
                      "Stop response transmission failed");
  return ESP_OK;
}

esp_err_t HttpService::set_mode_manual_handler(httpd_req_t* req) {
  set_header(req);
  control_state.control_mode = ControlMode::MANUAL;
  ESP_RETURN_ON_ERROR(httpd_resp_send(req, nullptr, 0),
                      HttpService::LOG_TAG,
                      "Stop response transmission failed");
  return ESP_OK;
}
esp_err_t HttpService::set_mode_automatic_handler(httpd_req_t* req) {
  set_header(req);
  control_state.control_mode = ControlMode::AUTO;
  ESP_RETURN_ON_ERROR(httpd_resp_send(req, nullptr, 0),
                      HttpService::LOG_TAG,
                      "Stop response transmission failed");
  return ESP_OK;
}
esp_err_t HttpService::set_mode_semi_automatic_handler(httpd_req_t* req) {
  set_header(req);
  control_state.control_mode = ControlMode::SEMI_AUTO;
  ESP_RETURN_ON_ERROR(httpd_resp_send(req, nullptr, 0),
                      HttpService::LOG_TAG,
                      "Stop response transmission failed");
  return ESP_OK;
}
esp_err_t HttpService::set_mode_smart_handler(httpd_req_t* req) {
  set_header(req);
  control_state.control_mode = ControlMode::SMART;
  ESP_RETURN_ON_ERROR(httpd_resp_send(req, nullptr, 0),
                      HttpService::LOG_TAG,
                      "Stop response transmission failed");
  return ESP_OK;
}

// esp_err_t HttpService::start_esp_cpu_usage_stream_handler(httpd_req_t* req) {
//   set_header(req);
//   ws_service.enable_stream(WsStream::ESP_TASK_DATA);
//   httpd_resp_send(req, nullptr, 0);
//   return ESP_OK;
// }
// esp_err_t HttpService::stop_esp_cpu_usage_stream_handler(httpd_req_t* req) {
//   set_header(req);
//   ws_service.disable_stream(WsStream::ESP_TASK_DATA);
//   httpd_resp_send(req, nullptr, 0);
//   return ESP_OK;
// }

esp_err_t HttpService::start_imu_data_stream_handler(httpd_req_t* req) {
  set_header(req);
  ws_service.enable_stream(WsStream::IMU_DATA);
  ESP_RETURN_ON_ERROR(httpd_resp_send(req, nullptr, 0),
                      HttpService::LOG_TAG,
                      "Start imu data stream failed");
  return ESP_OK;
}

esp_err_t HttpService::stop_imu_data_stream_handler(httpd_req_t* req) {
  set_header(req);
  ws_service.disable_stream(WsStream::IMU_DATA);
  ESP_RETURN_ON_ERROR(httpd_resp_send(req, nullptr, 0),
                      HttpService::LOG_TAG,
                      "Stop imu data stream failed");
  return ESP_OK;
}

esp_err_t HttpService::start_motor_data_stream_handler(httpd_req_t* req) {
  set_header(req);
  ws_service.enable_stream(WsStream::MOTOR_DATA);
  ESP_RETURN_ON_ERROR(httpd_resp_send(req, nullptr, 0),
                      HttpService::LOG_TAG,
                      "Start motor data stream failed");
  return ESP_OK;
}

esp_err_t HttpService::stop_motor_data_stream_handler(httpd_req_t* req) {
  set_header(req);
  ws_service.disable_stream(WsStream::MOTOR_DATA);
  ESP_RETURN_ON_ERROR(httpd_resp_send(req, nullptr, 0),
                      HttpService::LOG_TAG,
                      "Stop motor data stream failed");
  return ESP_OK;
}

esp_err_t HttpService::restart_handler(httpd_req_t* req) {
  set_header(req);
  ESP_RETURN_ON_ERROR(httpd_resp_send(req, nullptr, 0), HttpService::LOG_TAG, "Restart failed");
  Sync::sleep(10);
  esp_restart();
  return ESP_OK;
}

// esp_err_t HttpService::restart_stm32_handler(httpd_req_t* req) {
//   set_header(req);
//   send_command(SspCommand::RESTART);
//   uart_flush(config::stm_uart::port);
//   ESP_RETURN_ON_ERROR(httpd_resp_send(req, nullptr, 0), HttpService::LOG_TAG, "Restart failed");
//   return ESP_OK;
// }
// esp_err_t HttpService::restart_esp32_handler(httpd_req_t* req) {
//   set_header(req);
//   ESP_RETURN_ON_ERROR(httpd_resp_send(req, nullptr, 0), HttpService::LOG_TAG, "Restart failed");
//   vTaskDelay(10);
//   esp_restart();
//   return ESP_OK;
// }

esp_err_t HttpService::options_handler(httpd_req_t* req) {
  set_header(req);
  httpd_resp_send(req, NULL, 0);
  return ESP_OK;
}

esp_err_t HttpService::ws_data_post_handshake_handler(httpd_req_t* req) {
  auto client_fd = httpd_req_to_sockfd(req);
  ws_service.start_sending(client_fd);
  return ESP_OK;
}

esp_err_t HttpService::ws_data_handler(httpd_req_t* req) {
  httpd_ws_frame_t ws_frame;
  memset(&ws_frame, 0, sizeof(ws_frame));
  ws_frame.type = HTTPD_WS_TYPE_TEXT;
  httpd_ws_recv_frame(req, &ws_frame, 0);
  if (!ws_frame.len) {
    return ESP_OK;
  }
  ws_frame.payload = static_cast<uint8_t*>(malloc(ws_frame.len + 1));
  httpd_ws_recv_frame(req, &ws_frame, ws_frame.len);
  ws_frame.payload[ws_frame.len] = 0;
  auto data_result = JsonObject::parse(reinterpret_cast<char*>(ws_frame.payload));

  if (std::holds_alternative<JsonError>(data_result)) {
    free(ws_frame.payload);
    return ESP_OK;
  }

  auto data = std::get<JsonObject>(data_result);

  auto left_torque = data.get_number("leftTorque");
  auto right_torque = data.get_number("rightTorque");
  if (std::holds_alternative<double>(left_torque)) {
    control_state.control_params.manual.left.torque = std::get<double>(left_torque);
  }
  if (std::holds_alternative<double>(right_torque)) {
    control_state.control_params.manual.right.torque = std::get<double>(right_torque);
  }

  free(ws_frame.payload);
  return ESP_OK;
}

esp_err_t HttpService::register_http_uri(const char* uri_address,
                                         httpd_method_t method,
                                         esp_err_t (*handler)(httpd_req_t* req)) {
  httpd_uri uri = {
      .uri = uri_address,
      .method = method,
      .handler = handler,
      .user_ctx = nullptr,
      .is_websocket = false,
      .handle_ws_control_frames = false,
      .supported_subprotocol = nullptr,
      .ws_post_handshake_cb = nullptr,
  };
  ESP_RETURN_ON_ERROR(httpd_register_uri_handler(this->server_instance, &uri),
                      HttpService::LOG_TAG,
                      "Failed to register %s end point",
                      uri_address);
  return ESP_OK;
}

esp_err_t HttpService::register_http_uri_with_option(const char* uri_address,
                                                     httpd_method_t method,
                                                     esp_err_t (*handler)(httpd_req_t* req)) {
  httpd_uri uri = {
      .uri = uri_address,
      .method = method,
      .handler = handler,
      .user_ctx = nullptr,
      .is_websocket = false,
      .handle_ws_control_frames = false,
      .supported_subprotocol = nullptr,
      .ws_post_handshake_cb = nullptr,
  };

  httpd_uri option_uri = {
      .uri = uri_address,
      .method = HTTP_OPTIONS,
      .handler = HttpService::options_handler,
      .user_ctx = nullptr,
      .is_websocket = false,
      .handle_ws_control_frames = false,
      .supported_subprotocol = nullptr,
      .ws_post_handshake_cb = nullptr,
  };

  ESP_RETURN_ON_ERROR(httpd_register_uri_handler(this->server_instance, &uri),
                      HttpService::LOG_TAG,
                      "Failed to register %s end point",
                      uri_address);

  ESP_RETURN_ON_ERROR(httpd_register_uri_handler(this->server_instance, &option_uri),
                      HttpService::LOG_TAG,
                      "Failed to register option method for %s end point",
                      uri_address);
  return ESP_OK;
}

esp_err_t HttpService::register_ws_uri(const char* uri_address,
                                       esp_err_t (*handler)(httpd_req_t* req),
                                       esp_err_t (*post_handshake_handler)(httpd_req_t* req)) {
  httpd_uri_t uri = {
      .uri = uri_address,
      .method = HTTP_GET,
      .handler = handler,
      .user_ctx = nullptr,
      .is_websocket = true,
      .handle_ws_control_frames = false,
      .supported_subprotocol = nullptr,
      .ws_post_handshake_cb = post_handshake_handler,

  };
  ESP_RETURN_ON_ERROR(httpd_register_uri_handler(this->server_instance, &uri),
                      HttpService::LOG_TAG,
                      "Failed to register %s end point",
                      uri_address);
  return ESP_OK;
}

// // ! this needs to be async
// esp_err_t HttpService::check_for_update_handler(httpd_req_t* req) {
//   set_header(req);
//   esp_http_client_config_t config{};
//   config.url = "http://192.168.4.2:3000/latest-version";
//   config.method = HTTP_METHOD_GET;
//   config.timeout_ms = 5000;

//   esp_http_client_handle_t client = esp_http_client_init(&config);

//   uint32_t status_code = 200;
//   int resp_len;
//   std::string resp_str;
//   char* resp_buffer;
//   std::string status_code_str;
//   JsonObject error_json;

//   auto result = esp_http_client_open(client, 0);
//   if (result != ESP_OK) {
//     status_code = 500;
//     error_json.set("message", "could not reach server");
//     resp_str = error_json.stringify();
//     goto send;
//   }

//   result = esp_http_client_fetch_headers(client);
//   resp_len = esp_http_client_get_content_length(client);
//   resp_buffer = new char[resp_len + 1];
//   esp_http_client_read(client, resp_buffer, resp_len);
//   resp_buffer[resp_len] = 0;
//   resp_str = std::string(resp_buffer);
//   delete[] resp_buffer;
//   status_code = esp_http_client_get_status_code(client);

// send:
//   if (!error_json.is_empty()) {
//     resp_str = error_json.stringify();
//   }
//   status_code_str = std::format("{}", status_code);
//   httpd_resp_send_custom_err(req, status_code_str.c_str(), resp_str.c_str());
//   esp_http_client_close(client);
//   esp_http_client_cleanup(client);

//   return ESP_OK;
// }
// esp_err_t HttpService::get_version_handler(httpd_req_t* req) {
//   set_header(req);
//   JsonObject resp_json;
//   resp_json.set("api", API_VERSION);
//   resp_json.set("core", CORE_VERSION);
//   auto resp_str = resp_json.stringify();
//   httpd_resp_send(req, resp_str.c_str(), HTTPD_RESP_USE_STRLEN);
//   return ESP_OK;
// }

// esp_err_t HttpService::update_handler(httpd_req_t* req) {
//   ota_busy = true;
//   set_header(req);
//   httpd_req_t* async_req;
//   httpd_req_async_handler_begin(req, &async_req);
//   http_ota_handler_service.queue.send(async_req);
//   return ESP_OK;
// }

// esp_err_t HttpService::get_ota_status(httpd_req_t* req) {
//   set_header(req);
//   JsonObject resp_json;
//   resp_json.set("busy", ota_busy);
//   auto resp_str = resp_json.stringify();
//   httpd_resp_send(req, resp_str.c_str(), HTTPD_RESP_USE_STRLEN);
//   return ESP_OK;
// }

esp_err_t HttpService::set_automatic_control_params(httpd_req_t* req) {
  set_header(req);
  char* req_body = new char[req->content_len];
  httpd_req_recv(req, req_body, req->content_len);
  JsonObject resp_json;
  auto req_json_result = JsonObject::parse(req_body);
  delete[] req_body;

  if (std::holds_alternative<JsonError>(req_json_result)) {
    resp_json.set("message", "parse error");
    return send_json(req, resp_json, HTTPD_400_BAD_REQUEST);
  }

  auto req_json = std::get<JsonObject>(req_json_result);
  auto left_json_result = req_json.get_object("left", &resp_json);
  auto right_json_result = req_json.get_object("right", &resp_json);

  if (std::holds_alternative<JsonError>(left_json_result) ||
      std::holds_alternative<JsonError>(right_json_result)) {
    return send_json(req, resp_json, HTTPD_400_BAD_REQUEST);
  }

  auto left_json = std::get<JsonObject>(left_json_result);
  auto right_json = std::get<JsonObject>(right_json_result);

  auto left_timeout_result = left_json.get_number("timeout", &resp_json);
  auto left_torque_result = left_json.get_number("torque", &resp_json);
  auto left_velocity_threshold_result = left_json.get_number("velocityThreshold", &resp_json);
  auto right_timeout_result = right_json.get_number("timeout", &resp_json);
  auto right_torque_result = right_json.get_number("torque", &resp_json);
  auto right_velocity_threshold_result = right_json.get_number("velocityThreshold", &resp_json);

  if (std::holds_alternative<JsonError>(left_timeout_result) ||
      std::holds_alternative<JsonError>(left_torque_result) ||
      std::holds_alternative<JsonError>(left_velocity_threshold_result) ||
      std::holds_alternative<JsonError>(right_timeout_result) ||
      std::holds_alternative<JsonError>(right_torque_result) ||
      std::holds_alternative<JsonError>(right_velocity_threshold_result)) {
    return send_json(req, resp_json, HTTPD_400_BAD_REQUEST);
  }
  auto left_timeout = std::get<double>(left_timeout_result);
  auto left_torque = std::get<double>(left_torque_result);
  auto left_velocity_threshold = std::get<double>(left_velocity_threshold_result);
  auto right_timeout = std::get<double>(right_timeout_result);
  auto right_torque = std::get<double>(right_torque_result);
  auto right_velocity_threshold = std::get<double>(right_velocity_threshold_result);

  ESP_LOGI("PARAMS LEFT", "%f %f %f", left_timeout, left_torque, left_velocity_threshold);
  ESP_LOGI("PARAMS RIGHT", "%f %f %f", right_timeout, right_torque, right_velocity_threshold);

  control_state.control_params.automatic.left.timeout = left_timeout;
  control_state.control_params.automatic.left.torque = left_torque;
  control_state.control_params.automatic.left.velocity_threshold = left_velocity_threshold;
  control_state.control_params.automatic.right.timeout = right_timeout;
  control_state.control_params.automatic.right.torque = right_torque;
  control_state.control_params.automatic.right.velocity_threshold = right_velocity_threshold;

  httpd_resp_send(req, nullptr, 0);

  return ESP_OK;
}

esp_err_t HttpService::set_semiautomatic_control_params(httpd_req_t* req) {
  set_header(req);
  char* req_body = new char[req->content_len];
  httpd_req_recv(req, req_body, req->content_len);
  JsonObject resp_json;
  auto req_json_result = JsonObject::parse(req_body);

  if (std::holds_alternative<JsonError>(req_json_result)) {
    resp_json.set("message", "parse error");
    return send_json(req, resp_json, HTTPD_400_BAD_REQUEST);
  }

  auto req_json = std::get<JsonObject>(req_json_result);

  auto weak_leg_result = req_json.get_string("weakLeg", &resp_json);
  auto start_assist_angle_result = req_json.get_number("startAssistAngle", &resp_json);
  auto stop_assist_angle_result = req_json.get_number("stopAssistAngle", &resp_json);
  auto left_json_result = req_json.get_object("left", &resp_json);
  auto right_json_result = req_json.get_object("right", &resp_json);

  if (std::holds_alternative<JsonError>(weak_leg_result) ||
      std::holds_alternative<JsonError>(start_assist_angle_result) ||
      std::holds_alternative<JsonError>(stop_assist_angle_result) ||
      std::holds_alternative<JsonError>(left_json_result) ||
      std::holds_alternative<JsonError>(left_json_result)) {
    return send_json(req, resp_json, HTTPD_400_BAD_REQUEST);
  }

  auto weak_leg =
      std::strcmp(std::get<char*>(weak_leg_result), "left") == 0 ? Leg::LEFT : Leg::RIGHT;
  auto start_assist_angle = std::get<double>(start_assist_angle_result);
  auto stop_assist_angle = std::get<double>(stop_assist_angle_result);
  auto left_json = std::get<JsonObject>(left_json_result);
  auto right_json = std::get<JsonObject>(right_json_result);
  auto left_torque_result = left_json.get_number("torque", &resp_json);
  auto left_timeout_result = left_json.get_number("timeout", &resp_json);
  auto left_delay_result = left_json.get_number("delay", &resp_json);
  auto right_torque_result = right_json.get_number("torque", &resp_json);
  auto right_timeout_result = right_json.get_number("timeout", &resp_json);
  auto right_delay_result = right_json.get_number("delay", &resp_json);

  if (std::holds_alternative<JsonError>(left_torque_result) ||
      std::holds_alternative<JsonError>(left_delay_result) ||
      std::holds_alternative<JsonError>(left_timeout_result) ||
      std::holds_alternative<JsonError>(right_torque_result) ||
      std::holds_alternative<JsonError>(right_delay_result) ||
      std::holds_alternative<JsonError>(right_timeout_result)) {
    return send_json(req, resp_json, HTTPD_400_BAD_REQUEST);
  }

  auto left_torque = std::get<double>(left_torque_result);
  auto left_timeout = std::get<double>(left_timeout_result);
  auto left_delay = std::get<double>(left_delay_result);
  auto right_torque = std::get<double>(right_torque_result);
  auto right_timeout = std::get<double>(right_timeout_result);
  auto right_delay = std::get<double>(right_delay_result);

  control_state.control_params.semiautomatic.left.torque = left_torque;
  control_state.control_params.semiautomatic.left.timeout = left_timeout;
  control_state.control_params.semiautomatic.left.delay = left_delay;
  control_state.control_params.semiautomatic.right.torque = right_torque;
  control_state.control_params.semiautomatic.right.timeout = right_timeout;
  control_state.control_params.semiautomatic.right.delay = right_delay;
  control_state.control_params.semiautomatic.weak_leg = weak_leg;
  control_state.control_params.semiautomatic.start_assist_angle = start_assist_angle;
  control_state.control_params.semiautomatic.stop_assist_angle = stop_assist_angle;

  httpd_resp_send(req, nullptr, 0);
  return ESP_OK;
}

esp_err_t HttpService::set_smart_control_params(httpd_req_t* req) {
  set_header(req);
  char* req_body = new char[req->content_len];
  httpd_req_recv(req, req_body, req->content_len);
  JsonObject resp_json;
  auto req_json_result = JsonObject::parse(req_body);
  delete[] req_body;

  if (std::holds_alternative<JsonError>(req_json_result)) {
    resp_json.set("message", "parse error");
    return send_json(req, resp_json, HTTPD_400_BAD_REQUEST);
  }

  auto req_json = std::get<JsonObject>(req_json_result);

  auto left_json_result = req_json.get_object("left", &resp_json);
  auto right_json_result = req_json.get_object("right", &resp_json);

  if (std::holds_alternative<JsonError>(left_json_result) ||
      std::holds_alternative<JsonError>(right_json_result)) {
    return send_json(req, resp_json, HTTPD_400_BAD_REQUEST);
  }

  auto left_json = std::get<JsonObject>(left_json_result);
  auto right_json = std::get<JsonObject>(right_json_result);

  auto left_torque_result = left_json.get_number("torque");
  auto right_torque_result = right_json.get_number("torque");

  if (std::holds_alternative<JsonError>(left_torque_result) ||
      std::holds_alternative<JsonError>(right_torque_result)) {
    return send_json(req, resp_json, HTTPD_400_BAD_REQUEST);
  }
  auto left_torque = std::get<double>(left_torque_result);
  auto right_torque = std::get<double>(right_torque_result);
  control_state.control_params.smart.left.torque = left_torque;
  control_state.control_params.smart.right.torque = right_torque;
  httpd_resp_send(req, nullptr, 0);
  return ESP_OK;
}

esp_err_t HttpService::register_dynamic_endpoints() {
  this->register_http_uri("/api/null", HTTP_GET, HttpService::null_request_handler);
  // this->register_http_uri("/api/version", HTTP_GET, HttpService::get_version_handler);
  // this->register_http_uri("/api/update/status", HTTP_GET, HttpService::get_ota_status);
  // this->register_http_uri("/api/update/check", HTTP_GET, HttpService::check_for_update_handler);
  this->register_http_uri("/api/restart", HTTP_GET, HttpService::restart_handler);
  this->register_http_uri("/api/states", HTTP_GET, HttpService::get_state_handler);
  // this->register_http_uri("/api/stm_stack_size", HTTP_GET, HttpService::get_stm_task_stack_size);
  // this->register_http_uri("/api/esp_stack_size", HTTP_GET, HttpService::get_esp_task_stack_size);
  this->register_http_uri("/api/streams/start/imu",
                          HTTP_GET,
                          HttpService::start_imu_data_stream_handler);
  this->register_http_uri("/api/streams/stop/imu",
                          HTTP_GET,
                          HttpService::stop_imu_data_stream_handler);

  this->register_http_uri("/api/streams/start/motor",
                          HTTP_GET,
                          HttpService::start_motor_data_stream_handler);
  this->register_http_uri("/api/streams/stop/motor",
                          HTTP_GET,
                          HttpService::stop_motor_data_stream_handler);
  // this->register_http_uri("/api/streams/start/esp_cpu_usage",
  //                         HTTP_GET,
  //                         HttpService::start_esp_cpu_usage_stream_handler);
  // this->register_http_uri("/api/streams/stop/esp_cpu_usage",
  //                         HTTP_GET,
  //                         HttpService::stop_esp_cpu_usage_stream_handler);
  this->register_http_uri("/api/wifi/scan", HTTP_GET, HttpService::scan_wifi_handler);
  this->register_http_uri("/api/wifi", HTTP_GET, HttpService::get_connected_wifi);
  this->register_http_uri("/api/wifi/connect", HTTP_POST, HttpService::connect_to_wifi_handler);
  this->register_http_uri("/api/wifi/disconnect", HTTP_GET, HttpService::disconnect_wifi_handler);
  this->register_http_uri_with_option("/api/start", HTTP_PUT, HttpService::start_handler);
  this->register_http_uri_with_option("/api/stop", HTTP_PUT, HttpService::stop_handler);
  this->register_http_uri_with_option("/api/set-mode/manual",
                                      HTTP_PUT,
                                      HttpService::set_mode_manual_handler);
  this->register_http_uri_with_option("/api/set-mode/automatic",
                                      HTTP_PUT,
                                      HttpService::set_mode_automatic_handler);
  this->register_http_uri_with_option("/api/set-mode/semi-automatic",
                                      HTTP_PUT,
                                      HttpService::set_mode_semi_automatic_handler);
  this->register_http_uri_with_option("/api/set-mode/smart",
                                      HTTP_PUT,
                                      HttpService::set_mode_smart_handler);
  // this->register_http_uri_with_option("/api/date-time", HTTP_PUT, HttpService::set_rtc);
  this->register_http_uri_with_option("/api/automatic/params",
                                      HTTP_PUT,
                                      set_automatic_control_params);
  this->register_http_uri_with_option("/api/semiautomatic/params",
                                      HTTP_PUT,
                                      set_semiautomatic_control_params);
  this->register_http_uri_with_option("/api/smart/params", HTTP_PUT, set_smart_control_params);
  this->register_ws_uri("/api/data", HttpService::ws_data_handler, ws_data_post_handshake_handler);
  return ESP_OK;
}

void HttpService::start() {
  httpd_config_t http_config = HTTPD_DEFAULT_CONFIG();
  http_config.max_uri_handlers = 128;
  ESP_ERROR_CHECK(httpd_start(&server_instance, &http_config));
  http_server_register_assets(server_instance);
  ESP_ERROR_CHECK(register_dynamic_endpoints());
}
