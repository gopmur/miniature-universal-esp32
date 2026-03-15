#include <cmath>
#include <cstdio>
#include <variant>

#include "services/http.hpp"

#include "config.hpp"
#include "driver/uart.h"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_wifi_types_generic.h"
#include "helper.hpp"
#include "helper/json.hpp"
#include "http_assets.hpp"
#include "http_parser.h"

#include "context/services/http.hpp"
#include "context/services/http_async_handler.hpp"
#include "context/services/ws.hpp"
#include "helper/uart.hpp"
#include "portmacro.h"
#include "services/http_async_handler.hpp"
#include "services/stm_uart/rssp.hpp"

const char* get_contorl_mode_str(ControlMode control_mode) {
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
}

// esp_err_t HttpService::get_session_reports_handler(httpd_req_t* req) {
//   constexpr int record_count = 8;
//   constexpr int data_count = 2000;
//   constexpr float sampling_rate = 0.1;

//   httpd_resp_set_type(req, "application/octet-stream");
//   httpd_resp_set_hdr(req, "Transfer-Encoding", "chunked");

//   ReportRecord buffer[record_count];
//   ReportRecord record;
//   int buffer_index = 0;

//   for (int i = 0; i < data_count; i++) {
//     record.callback_time += 100000;
//     record.smc_mr_vel = std::sin(record.callback_time);
//     record.smc_ml_vel = std::sin(record.callback_time);
//     record.smc_mr_pos += record.smc_mr_vel * sampling_rate;
//     record.smc_ml_pos += record.smc_ml_vel * sampling_rate;

//     buffer[buffer_index] = record;
//     buffer_index++;

//     if (buffer_index == record_count) {
//       buffer_index = 0;
//       ESP_RETURN_ON_ERROR(httpd_resp_send_chunk(req,
//                                                 reinterpret_cast<char*>(buffer),
//                                                 sizeof(buffer)),
//                           HttpService::LOG_TAG,
//                           "Chunk transmission failed.");
//     }
//   };
//   if (buffer_index != 0) {
//     ESP_RETURN_ON_ERROR(
//         httpd_resp_send_chunk(req,
//                               reinterpret_cast<char*>(buffer),
//                               sizeof(ReportRecord) * buffer_index),
//         HttpService::LOG_TAG,
//         "Chunk transmission failed");
//   };
//   ESP_RETURN_ON_ERROR(httpd_resp_send_chunk(req, nullptr, 0),
//                       HttpService::LOG_TAG,
//                       "Termination of chunk transmission failed");
//   return ESP_OK;
// }

esp_err_t HttpService::scan_wifi_handler(httpd_req_t* req) {
  allow_cors(req);
  httpd_req_t* async_req;
  httpd_req_async_handler_begin(req, &async_req);
  HttpAsyncHandlerMessage http_async_handler_message = {
      .type = HttpAsyncHandlerMessageType::SCAN_WIFI,
      .req = async_req,
  };
  http_async_handler_service.queue.send(http_async_handler_message,
                                        portMAX_DELAY);
  return ESP_OK;
}

esp_err_t HttpService::connect_to_wifi_handler(httpd_req_t* req) {
  return ESP_OK;
}

void HttpService::set_rtc_time(Json* time_json, Json* time_error_json) {
  auto hours_item = time_json->get_number("hours", time_error_json);
  auto minutes_item = time_json->get_number("minutes", time_error_json);
  auto seconds_item = time_json->get_number("seconds", time_error_json);
  if (std::holds_alternative<JsonError>(hours_item) ||
      std::holds_alternative<JsonError>(minutes_item) ||
      std::holds_alternative<JsonError>(seconds_item)) {
    return;
  }
  uint8_t hours = std::get<double>(hours_item);
  uint8_t minutes = std::get<double>(minutes_item);
  uint8_t seconds = std::get<double>(seconds_item);
  std::array<uint8_t, 4> time_data = {hours, minutes, seconds, 0};
  write_address(RsspAddress::RTC_TIME, time_data);
}

void HttpService::set_rtc_date(Json* date_json, Json* date_error_json) {
  auto year_item = date_json->get_number("year", date_error_json);
  auto month_item = date_json->get_number("month", date_error_json);
  auto day_item = date_json->get_number("day", date_error_json);
  if (std::holds_alternative<JsonError>(year_item) ||
      std::holds_alternative<JsonError>(month_item) ||
      std::holds_alternative<JsonError>(day_item)) {
    return;
  }
  uint16_t year = std::get<double>(year_item);
  uint8_t month = std::get<double>(month_item);
  uint8_t day = std::get<double>(day_item);
  std::array<uint8_t, 4> date_data = {get_byte(year, 1),
                                      get_byte(year, 0),
                                      month,
                                      day};
  write_address(RsspAddress::RTC_DATE, date_data);
}

esp_err_t HttpService::set_rtc(httpd_req_t* req) {
  allow_cors(req);
  char* req_body = static_cast<char*>(malloc(req->content_len + 1));
  int received = httpd_req_recv(req, req_body, req->content_len);
  req_body[received] = 0;

  Json res_json;
  Json req_json(req_body);
  free(req_body);

  auto time_item = req_json.get_object("time", &res_json);
  auto date_item = req_json.get_object("date", &res_json);

  if (std::holds_alternative<Json>(time_item)) {
    Json time_json = std::get<Json>(time_item);
    Json time_error_json;
    set_rtc_time(&time_json, &time_error_json);
    if (!time_error_json.is_empty()) {
      res_json.set_object("time", &time_error_json);
    }
  }

  if (std::holds_alternative<Json>(date_item)) {
    Json date_json = std::get<Json>(date_item);
    Json date_error_json;
    set_rtc_date(&date_json, &date_error_json);
    if (!date_error_json.is_empty()) {
      res_json.set_object("date", &date_error_json);
    }
  }

  auto res_str = res_json.stringify();
  if (res_json.is_empty()) {
    httpd_resp_send(req, res_str, strlen(res_str));
  } else {
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, res_str);
  }

  free(res_str);

  return ESP_OK;
}

void HttpService::allow_cors(httpd_req_t* req) {
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  httpd_resp_set_hdr(req,
                     "Access-Control-Allow-Methods",
                     "GET, PUT, POST, OPTIONS");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "Content-Type");
}

esp_err_t HttpService::get_state_handler(httpd_req_t* req) {
  HttpService::allow_cors(req);
  http_service.queue.flush();

  read_addresses({RsspAddress::RUNNING,
                  RsspAddress::RIGHT_TORQUE,
                  RsspAddress::LEFT_TORQUE,
                  RsspAddress::CONTROL_MODE});

  Json res_json;

  for (int i = 0; i < 4; i++) {
    auto response = http_service.queue.receive(200);
    if (!response.has_value()) {
      httpd_resp_send_err(req,
                          HTTPD_500_INTERNAL_SERVER_ERROR,
                          "STM32 not responding");
      return ESP_OK;
    }
    switch (response->header) {
      case HttpQueueMessageHeader::RUNNING:
        res_json.set_bool("running", response->payload.b);
        break;
      case HttpQueueMessageHeader::LEFT_MANUAL_TORQUE:
        res_json.set_number("leftTorque", response->payload.f);
        break;
      case HttpQueueMessageHeader::RIGHT_MANUAL_TORQUE:
        res_json.set_number("rightTorque", response->payload.f);
        break;
      case HttpQueueMessageHeader::MODE:
        res_json.set_string(
            "mode",
            get_contorl_mode_str(response->payload.control_mode));

        break;
    }
  }

  auto json_str = res_json.stringify();
  httpd_resp_set_type(req, "application/json");
  auto ret = httpd_resp_send(req, json_str, HTTPD_RESP_USE_STRLEN);
  if (ret != ESP_OK) {
    ESP_LOGE(HttpService::LOG_TAG, "Get running response transmission failed");
  }
  free(json_str);
  return ESP_OK;
}

esp_err_t HttpService::start_handler(httpd_req_t* req) {
  HttpService::allow_cors(req);
  write_address(RsspAddress::RUNNING, true);
  ESP_RETURN_ON_ERROR(httpd_resp_send(req, nullptr, 0),
                      HttpService::LOG_TAG,
                      "Start response transmission failed");
  return ESP_OK;
}

esp_err_t HttpService::stop_handler(httpd_req_t* req) {
  HttpService::allow_cors(req);
  write_address(RsspAddress::RUNNING, false);
  ESP_RETURN_ON_ERROR(httpd_resp_send(req, nullptr, 0),
                      HttpService::LOG_TAG,
                      "Stop response transmission failed");
  return ESP_OK;
}
esp_err_t HttpService::set_right_torque_handler(httpd_req_t* req) {
  HttpService::allow_cors(req);
  int len = req->content_len;
  char* body = static_cast<char*>(malloc(len + 1));

  int received = httpd_req_recv(req, body, len);
  body[received] = 0;

  Json res_json;
  Json req_json(body);
  free(body);

  auto torque = req_json.get_number("torque", &res_json);

  if (std::holds_alternative<JsonError>(torque)) {
    auto res_str = res_json.stringify();
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, res_str);
    free(res_str);
    return ESP_OK;
  }

  write_address(RsspAddress::RIGHT_TORQUE,
                static_cast<float>(std::get<double>(torque)));

  ESP_RETURN_ON_ERROR(httpd_resp_send(req, nullptr, 0),
                      HttpService::LOG_TAG,
                      "Stop response transmission failed");
  return ESP_OK;
}

esp_err_t HttpService::set_left_torque_handler(httpd_req_t* req) {
  HttpService::allow_cors(req);
  int len = req->content_len;
  char* body = static_cast<char*>(malloc(len + 1));

  int received = httpd_req_recv(req, body, len);
  body[received] = 0;

  Json res_json;
  Json req_json(body);
  free(body);

  auto torque = req_json.get_number("torque", &res_json);

  if (std::holds_alternative<JsonError>(torque)) {
    auto res_str = res_json.stringify();
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, res_str);
    free(res_str);
    return ESP_OK;
  }

  write_address(RsspAddress::LEFT_TORQUE,
                static_cast<float>(std::get<double>(torque)));

  ESP_RETURN_ON_ERROR(httpd_resp_send(req, nullptr, 0),
                      HttpService::LOG_TAG,
                      "Stop response transmission failed");
  return ESP_OK;
}

esp_err_t HttpService::set_mode_manual_handler(httpd_req_t* req) {
  HttpService::allow_cors(req);
  write_address(RsspAddress::CONTROL_MODE,
                static_cast<uint8_t>(ControlMode::MANUAL));
  ESP_RETURN_ON_ERROR(httpd_resp_send(req, nullptr, 0),
                      HttpService::LOG_TAG,
                      "Stop response transmission failed");
  return ESP_OK;
}
esp_err_t HttpService::set_mode_automatic_handler(httpd_req_t* req) {
  HttpService::allow_cors(req);
  write_address(RsspAddress::CONTROL_MODE,
                static_cast<uint8_t>(ControlMode::AUTO));
  ESP_RETURN_ON_ERROR(httpd_resp_send(req, nullptr, 0),
                      HttpService::LOG_TAG,
                      "Stop response transmission failed");
  return ESP_OK;
}
esp_err_t HttpService::set_mode_semi_automatic_handler(httpd_req_t* req) {
  HttpService::allow_cors(req);
  write_address(RsspAddress::CONTROL_MODE,
                static_cast<uint8_t>(ControlMode::SEMI_AUTO));
  ESP_RETURN_ON_ERROR(httpd_resp_send(req, nullptr, 0),
                      HttpService::LOG_TAG,
                      "Stop response transmission failed");
  return ESP_OK;
}
esp_err_t HttpService::set_mode_smart_handler(httpd_req_t* req) {
  HttpService::allow_cors(req);
  write_address(RsspAddress::CONTROL_MODE,
                static_cast<uint8_t>(ControlMode::SMART));

  ESP_RETURN_ON_ERROR(httpd_resp_send(req, nullptr, 0),
                      HttpService::LOG_TAG,
                      "Stop response transmission failed");
  return ESP_OK;
}

esp_err_t HttpService::start_stm_cpu_usage_stream_handler(httpd_req_t* req) {
  HttpService::allow_cors(req);
  ws_service.enable_stm_cpu_usage();
  start_streams({
      RsspAddress::LED_SERVICE_CPU_USAGE,
      RsspAddress::IMU_SERVICE_CPU_USAGE,
      RsspAddress::MOTOR_SERVICE_CPU_USAGE,
      RsspAddress::SD_SERVICE_CPU_USAGE,
      RsspAddress::CAN_RECV_SERVICE_CPU_USAGE,
      RsspAddress::ESP_UART_RX_SERVICE_CPU_USAGE,
      RsspAddress::ESP_UART_TX_SERVICE_CPU_USAGE,
      RsspAddress::MONITOR_SERVICE_CPU_USAGE,
  });
  ESP_RETURN_ON_ERROR(httpd_resp_send(req, nullptr, 0),
                      HttpService::LOG_TAG,
                      "Start CPU usage stream failed");
  return ESP_OK;
}

esp_err_t HttpService::stop_stm_cpu_usage_stream_handler(httpd_req_t* req) {
  HttpService::allow_cors(req);
  ws_service.disable_stm_cpu_usage();
  stop_streams({
      RsspAddress::LED_SERVICE_CPU_USAGE,
      RsspAddress::IMU_SERVICE_CPU_USAGE,
      RsspAddress::MOTOR_SERVICE_CPU_USAGE,
      RsspAddress::SD_SERVICE_CPU_USAGE,
      RsspAddress::CAN_RECV_SERVICE_CPU_USAGE,
      RsspAddress::ESP_UART_RX_SERVICE_CPU_USAGE,
      RsspAddress::ESP_UART_TX_SERVICE_CPU_USAGE,
      RsspAddress::MONITOR_SERVICE_CPU_USAGE,
  });
  ESP_RETURN_ON_ERROR(httpd_resp_send(req, nullptr, 0),
                      HttpService::LOG_TAG,
                      "Stop CPU usage stream failed");
  return ESP_OK;
}

esp_err_t HttpService::start_esp_cpu_usage_stream_handler(httpd_req_t* req) {
  HttpService::allow_cors(req);
  ws_service.enable_esp_cpu_usage();
  httpd_resp_send(req, nullptr, 0);
  return ESP_OK;
}
esp_err_t HttpService::stop_esp_cpu_usage_stream_handler(httpd_req_t* req) {
  HttpService::allow_cors(req);
  ws_service.disable_esp_cpu_usage();
  httpd_resp_send(req, nullptr, 0);
  return ESP_OK;
}

esp_err_t HttpService::start_imu_data_stream_handler(httpd_req_t* req) {
  HttpService::allow_cors(req);
  ws_service.enable_imu_data();
  start_streams(
      {RsspAddress::IMU_GX, RsspAddress::IMU_GY, RsspAddress::IMU_GZ});
  ESP_RETURN_ON_ERROR(httpd_resp_send(req, nullptr, 0),
                      HttpService::LOG_TAG,
                      "Start imu data stream failed");
  return ESP_OK;
}
esp_err_t HttpService::stop_imu_data_stream_handler(httpd_req_t* req) {
  HttpService::allow_cors(req);
  ws_service.disable_imu_data();
  stop_streams({RsspAddress::IMU_GX, RsspAddress::IMU_GY, RsspAddress::IMU_GZ});
  ESP_RETURN_ON_ERROR(httpd_resp_send(req, nullptr, 0),
                      HttpService::LOG_TAG,
                      "Stop imu data stream failed");
  return ESP_OK;
}

esp_err_t HttpService::restart_handler(httpd_req_t* req) {
  HttpService::allow_cors(req);
  send_command(RsspCommand::RESTART);
  uart_flush(config::stm_uart::port);
  esp_restart();
  ESP_RETURN_ON_ERROR(httpd_resp_send(req, nullptr, 0),
                      HttpService::LOG_TAG,
                      "Restart failed");
  return ESP_OK;
}

esp_err_t HttpService::options_handler(httpd_req_t* req) {
  HttpService::allow_cors(req);
  httpd_resp_send(req, NULL, 0);
  return ESP_OK;
}

esp_err_t HttpService::ws_data_handler(httpd_req_t* req) {
  if (req->method == HTTP_GET) {
    auto client_fd = httpd_req_to_sockfd(req);
    ws_service.start_sending(client_fd);
    return ESP_OK;
  }
  httpd_ws_frame_t ws_frame;
  memset(&ws_frame, 0, sizeof(ws_frame));
  ws_frame.type = HTTPD_WS_TYPE_TEXT;
  httpd_ws_recv_frame(req, &ws_frame, 0);
  if (ws_frame.len) {
    ws_frame.payload = static_cast<uint8_t*>(malloc(ws_frame.len + 1));
    httpd_ws_recv_frame(req, &ws_frame, ws_frame.len);
    ws_frame.payload[ws_frame.len] = 0;

    Json data(reinterpret_cast<char*>(ws_frame.payload));
    auto left_torque = data.get_number("leftTorque");
    auto right_torque = data.get_number("rightTorque");
    if (std::holds_alternative<double>(left_torque)) {
      write_address(RsspAddress::LEFT_TORQUE,
                    static_cast<float>(std::get<double>(left_torque)));
    }
    if (std::holds_alternative<double>(right_torque)) {
      write_address(RsspAddress::RIGHT_TORQUE,
                    static_cast<float>(std::get<double>(right_torque)));
    }

    free(ws_frame.payload);
  }
  return ESP_OK;
}

esp_err_t HttpService::register_http_uri(
    const char* uri_address,
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
  };
  ESP_RETURN_ON_ERROR(httpd_register_uri_handler(this->server_instance, &uri),
                      HttpService::LOG_TAG,
                      "Failed to register %s end point",
                      uri_address);
  return ESP_OK;
}

esp_err_t HttpService::register_http_uri_with_option(
    const char* uri_address,
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
  };

  httpd_uri option_uri = {
      .uri = uri_address,
      .method = HTTP_OPTIONS,
      .handler = HttpService::options_handler,
      .user_ctx = nullptr,
      .is_websocket = false,
      .handle_ws_control_frames = false,
      .supported_subprotocol = nullptr,
  };

  ESP_RETURN_ON_ERROR(httpd_register_uri_handler(this->server_instance, &uri),
                      HttpService::LOG_TAG,
                      "Failed to register %s end point",
                      uri_address);

  ESP_RETURN_ON_ERROR(
      httpd_register_uri_handler(this->server_instance, &option_uri),
      HttpService::LOG_TAG,
      "Failed to register option method for %s end point",
      uri_address);
  return ESP_OK;
}

esp_err_t HttpService::register_ws_uri(const char* uri_address,
                                       esp_err_t (*handler)(httpd_req_t* req)) {
  httpd_uri_t uri = {
      .uri = uri_address,
      .method = HTTP_GET,
      .handler = handler,
      .user_ctx = nullptr,
      .is_websocket = true,
      .handle_ws_control_frames = false,
      .supported_subprotocol = nullptr,

  };
  ESP_RETURN_ON_ERROR(httpd_register_uri_handler(this->server_instance, &uri),
                      HttpService::LOG_TAG,
                      "Failed to register %s end point",
                      uri_address);
  return ESP_OK;
}

esp_err_t HttpService::register_dynamic_endpoints() {
  this->register_http_uri("/api/restart",
                          HTTP_GET,
                          HttpService::restart_handler);
  this->register_http_uri("/api/states",
                          HTTP_GET,
                          HttpService::get_state_handler);
  this->register_http_uri("/api/streams/start/imu",
                          HTTP_GET,
                          HttpService::start_imu_data_stream_handler);
  this->register_http_uri("/api/streams/stop/imu",
                          HTTP_GET,
                          HttpService::stop_imu_data_stream_handler);
  this->register_http_uri("/api/streams/start/stm_cpu_usage",
                          HTTP_GET,
                          HttpService::start_stm_cpu_usage_stream_handler);
  this->register_http_uri("/api/streams/stop/stm_cpu_usage",
                          HTTP_GET,
                          HttpService::stop_stm_cpu_usage_stream_handler);
  this->register_http_uri("/api/streams/start/esp_cpu_usage",
                          HTTP_GET,
                          HttpService::start_esp_cpu_usage_stream_handler);
  this->register_http_uri("/api/streams/stop/esp_cpu_usage",
                          HTTP_GET,
                          HttpService::stop_esp_cpu_usage_stream_handler);
  this->register_http_uri("/api/wifi/scan",
                          HTTP_GET,
                          HttpService::scan_wifi_handler);
  this->register_http_uri_with_option("/api/start",
                                      HTTP_PUT,
                                      HttpService::start_handler);
  this->register_http_uri_with_option("/api/stop",
                                      HTTP_PUT,
                                      HttpService::stop_handler);
  this->register_http_uri_with_option("/api/right_torque",
                                      HTTP_PUT,
                                      HttpService::set_right_torque_handler);
  this->register_http_uri_with_option("/api/left_torque",
                                      HTTP_PUT,
                                      HttpService::set_left_torque_handler);
  this->register_http_uri_with_option("/api/set-mode/manual",
                                      HTTP_PUT,
                                      HttpService::set_mode_manual_handler);
  this->register_http_uri_with_option("/api/set-mode/automatic",
                                      HTTP_PUT,
                                      HttpService::set_mode_automatic_handler);
  this->register_http_uri_with_option(
      "/api/set-mode/semi-automatic",
      HTTP_PUT,
      HttpService::set_mode_semi_automatic_handler);
  this->register_http_uri_with_option("/api/set-mode/smart",
                                      HTTP_PUT,
                                      HttpService::set_mode_smart_handler);
  this->register_http_uri_with_option("/api/date-time",
                                      HTTP_PUT,
                                      HttpService::set_rtc);
  this->register_ws_uri("/api/data", HttpService::ws_data_handler);
  return ESP_OK;
}

void HttpService::start() {
  httpd_config_t http_config = HTTPD_DEFAULT_CONFIG();
  http_config.max_uri_handlers = 32;
  ESP_ERROR_CHECK(httpd_start(&server_instance, &http_config));
  http_server_register_assets(server_instance);
  ESP_ERROR_CHECK(register_dynamic_endpoints());
}
