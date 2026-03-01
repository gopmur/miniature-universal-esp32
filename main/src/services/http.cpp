#include <cmath>
#include <cstdio>

#include "services/http.hpp"

#include "config.hpp"
#include "driver/uart.h"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_system.h"
#include "http_assets.hpp"
#include "http_parser.h"
#include "portmacro.h"
#include "report.hpp"

#include <cJSON.h>
#include "context.hpp"
#include "services/helper/uart.hpp"
#include "services/stm_uart/lappl.hpp"
#include "services/stm_uart/packet.hpp"

esp_err_t HttpService::get_session_reports_handler(httpd_req_t* req) {
  constexpr int record_count = 8;
  constexpr int data_count = 2000;
  constexpr float sampling_rate = 0.1;

  httpd_resp_set_type(req, "application/octet-stream");
  httpd_resp_set_hdr(req, "Transfer-Encoding", "chunked");

  ReportRecord buffer[record_count];
  ReportRecord record;
  int buffer_index = 0;

  for (int i = 0; i < data_count; i++) {
    record.callback_time += 100000;
    record.smc_mr_vel = std::sin(record.callback_time);
    record.smc_ml_vel = std::sin(record.callback_time);
    record.smc_mr_pos += record.smc_mr_vel * sampling_rate;
    record.smc_ml_pos += record.smc_ml_vel * sampling_rate;

    buffer[buffer_index] = record;
    buffer_index++;

    if (buffer_index == record_count) {
      buffer_index = 0;
      ESP_RETURN_ON_ERROR(httpd_resp_send_chunk(req,
                                                reinterpret_cast<char*>(buffer),
                                                sizeof(buffer)),
                          HttpService::LOG_TAG,
                          "Chunk transmission failed.");
    }
  };
  if (buffer_index != 0) {
    ESP_RETURN_ON_ERROR(
        httpd_resp_send_chunk(req,
                              reinterpret_cast<char*>(buffer),
                              sizeof(ReportRecord) * buffer_index),
        HttpService::LOG_TAG,
        "Chunk transmission failed");
  };
  ESP_RETURN_ON_ERROR(httpd_resp_send_chunk(req, nullptr, 0),
                      HttpService::LOG_TAG,
                      "Termination of chunk transmission failed");
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
  context::http_service.queue.flush();

  read_addresses({LapplAddress::RUNNING,
                  LapplAddress::RIGHT_TORQUE,
                  LapplAddress::LEFT_TORQUE,
                  LapplAddress::CONTROL_MODE});

  auto root = cJSON_CreateObject();

  for (int i = 0; i < 4; i++) {
    auto response = context::http_service.queue.receive(200);
    if (!response.has_value()) {
      cJSON_Delete(root);
      httpd_resp_send_err(req,
                          HTTPD_500_INTERNAL_SERVER_ERROR,
                          "STM32 not responding");
      return ESP_OK;
    }
    switch (response->header) {
      case HttpQueueMessageHeader::RUNNING:
        cJSON_AddBoolToObject(root, "running", response->payload.b);
        break;
      case HttpQueueMessageHeader::LEFT_MANUAL_TORQUE:
        cJSON_AddNumberToObject(root, "leftTorque", response->payload.f);
        break;
      case HttpQueueMessageHeader::RIGHT_MANUAL_TORQUE:
        cJSON_AddNumberToObject(root, "rightTorque", response->payload.f);
        break;
      case HttpQueueMessageHeader::MODE:
        cJSON_AddStringToObject(
            root,
            "mode",
            get_contorl_mode_str(response->payload.control_mode));
        break;
    }
  }

  auto json_str = cJSON_PrintUnformatted(root);
  cJSON_Delete(root);
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
  write_address(LapplAddress::RUNNING, true);
  ESP_RETURN_ON_ERROR(httpd_resp_send(req, nullptr, 0),
                      HttpService::LOG_TAG,
                      "Start response transmission failed");
  return ESP_OK;
}

esp_err_t HttpService::stop_handler(httpd_req_t* req) {
  HttpService::allow_cors(req);
  write_address(LapplAddress::RUNNING, false);
  ESP_RETURN_ON_ERROR(httpd_resp_send(req, nullptr, 0),
                      HttpService::LOG_TAG,
                      "Stop response transmission failed");
  return ESP_OK;
}
esp_err_t HttpService::set_right_torque_handler(httpd_req_t* req) {
  HttpService::allow_cors(req);
  int len = req->content_len;
  char* body = (char*)malloc(len + 1);

  int received = httpd_req_recv(req, body, len);
  body[received] = 0;

  cJSON* root = cJSON_Parse(body);
  cJSON* torqueItem = cJSON_GetObjectItem(root, "torque");

  if (cJSON_IsNumber(torqueItem)) {
    float torque = torqueItem->valuedouble;
    write_address(LapplAddress::RIGHT_TORQUE, torque);
  }

  cJSON_Delete(root);
  free(body);

  ESP_RETURN_ON_ERROR(httpd_resp_send(req, nullptr, 0),
                      HttpService::LOG_TAG,
                      "Stop response transmission failed");
  return ESP_OK;
}

esp_err_t HttpService::set_left_torque_handler(httpd_req_t* req) {
  HttpService::allow_cors(req);
  int len = req->content_len;
  char* body = (char*)malloc(len + 1);

  int received = httpd_req_recv(req, body, len);
  body[received] = 0;

  cJSON* root = cJSON_Parse(body);
  cJSON* torqueItem = cJSON_GetObjectItem(root, "torque");

  if (cJSON_IsNumber(torqueItem)) {
    float torque = torqueItem->valuedouble;
    write_address(LapplAddress::LEFT_TORQUE, torque);
  }

  cJSON_Delete(root);
  free(body);

  ESP_RETURN_ON_ERROR(httpd_resp_send(req, nullptr, 0),
                      HttpService::LOG_TAG,
                      "Stop response transmission failed");

  return ESP_OK;
}

esp_err_t HttpService::set_mode_manual_handler(httpd_req_t* req) {
  HttpService::allow_cors(req);
  write_address(LapplAddress::CONTROL_MODE,
                static_cast<uint8_t>(ControlMode::MANUAL));
  ESP_RETURN_ON_ERROR(httpd_resp_send(req, nullptr, 0),
                      HttpService::LOG_TAG,
                      "Stop response transmission failed");
  return ESP_OK;
}
esp_err_t HttpService::set_mode_automatic_handler(httpd_req_t* req) {
  HttpService::allow_cors(req);
  write_address(LapplAddress::CONTROL_MODE,
                static_cast<uint8_t>(ControlMode::AUTO));
  ESP_RETURN_ON_ERROR(httpd_resp_send(req, nullptr, 0),
                      HttpService::LOG_TAG,
                      "Stop response transmission failed");
  return ESP_OK;
}
esp_err_t HttpService::set_mode_semi_automatic_handler(httpd_req_t* req) {
  HttpService::allow_cors(req);
  write_address(LapplAddress::CONTROL_MODE,
                static_cast<uint8_t>(ControlMode::SEMI_AUTO));
  ESP_RETURN_ON_ERROR(httpd_resp_send(req, nullptr, 0),
                      HttpService::LOG_TAG,
                      "Stop response transmission failed");
  return ESP_OK;
}
esp_err_t HttpService::set_mode_smart_handler(httpd_req_t* req) {
  HttpService::allow_cors(req);
  write_address(LapplAddress::CONTROL_MODE,
                static_cast<uint8_t>(ControlMode::SMART));

  ESP_RETURN_ON_ERROR(httpd_resp_send(req, nullptr, 0),
                      HttpService::LOG_TAG,
                      "Stop response transmission failed");
  return ESP_OK;
}

esp_err_t HttpService::start_cpu_usage_stream_handler(httpd_req_t* req) {
  HttpService::allow_cors(req);
  start_streams({
      LapplAddress::LED_SERVICE_CPU_USAGE,
      LapplAddress::IMU_SERVICE_CPU_USAGE,
      LapplAddress::MOTOR_SERVICE_CPU_USAGE,
      LapplAddress::SD_SERVICE_CPU_USAGE,
      LapplAddress::CAN_RECV_SERVICE_CPU_USAGE,
      LapplAddress::ESP_UART_RX_SERVICE_CPU_USAGE,
      LapplAddress::ESP_UART_TX_SERVICE_CPU_USAGE,
      LapplAddress::MONITOR_SERVICE_CPU_USAGE,
  });
  ESP_RETURN_ON_ERROR(httpd_resp_send(req, nullptr, 0),
                      HttpService::LOG_TAG,
                      "Start CPU usage stream failed");
  return ESP_OK;
}
esp_err_t HttpService::stop_cpu_usage_stream_handler(httpd_req_t* req) {
  HttpService::allow_cors(req);
  stop_streams({
      LapplAddress::LED_SERVICE_CPU_USAGE,
      LapplAddress::IMU_SERVICE_CPU_USAGE,
      LapplAddress::MOTOR_SERVICE_CPU_USAGE,
      LapplAddress::SD_SERVICE_CPU_USAGE,
      LapplAddress::CAN_RECV_SERVICE_CPU_USAGE,
      LapplAddress::ESP_UART_RX_SERVICE_CPU_USAGE,
      LapplAddress::ESP_UART_TX_SERVICE_CPU_USAGE,
      LapplAddress::MONITOR_SERVICE_CPU_USAGE,
  });
  ESP_RETURN_ON_ERROR(httpd_resp_send(req, nullptr, 0),
                      HttpService::LOG_TAG,
                      "Stop CPU usage stream failed");
  return ESP_OK;
}

esp_err_t HttpService::start_imu_data_stream_handler(httpd_req_t* req) {
  HttpService::allow_cors(req);
  start_streams(
      {LapplAddress::IMU_GX, LapplAddress::IMU_GY, LapplAddress::IMU_GZ});
  ESP_RETURN_ON_ERROR(httpd_resp_send(req, nullptr, 0),
                      HttpService::LOG_TAG,
                      "Start imu data stream failed");
  return ESP_OK;
}
esp_err_t HttpService::stop_imu_data_stream_handler(httpd_req_t* req) {
  HttpService::allow_cors(req);
  stop_streams(
      {LapplAddress::IMU_GX, LapplAddress::IMU_GY, LapplAddress::IMU_GZ});
  ESP_RETURN_ON_ERROR(httpd_resp_send(req, nullptr, 0),
                      HttpService::LOG_TAG,
                      "Stop imu data stream failed");
  return ESP_OK;
}

esp_err_t HttpService::restart_handler(httpd_req_t* req) {
  HttpService::allow_cors(req);
  send_command(LapplCommand::RESTART);
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
    context::ws_service.start_sending(client_fd);
    ESP_LOGI("WS", "WebSocket client connected, fd=%d", 0);
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
  this->register_http_uri("/api/streams/start/cpu_usage",
                          HTTP_GET,
                          HttpService::start_cpu_usage_stream_handler);
  this->register_http_uri("/api/streams/stop/cpu_usage",
                          HTTP_GET,
                          HttpService::stop_cpu_usage_stream_handler);
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
