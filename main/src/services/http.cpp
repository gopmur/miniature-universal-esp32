#include <cmath>
#include <cstdio>

#include "services/http.hpp"

#include "config.hpp"
#include "driver/uart.h"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "http_assets.hpp"
#include "http_parser.h"
#include "portmacro.h"
#include "report.hpp"

#include <cJSON.h>
#include "context.hpp"
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
  auto uart_packet =
      LapplPacket::make_read_packet(LapplAddress::RUNNING).get_raw_packet();
  ESP_LOGI("UART",
           "%02x %02x %02x %02x %02x %02x %02x %02x",
           uart_packet.data()[0],
           uart_packet.data()[1],
           uart_packet.data()[2],
           uart_packet.data()[3],
           uart_packet.data()[4],
           uart_packet.data()[5],
           uart_packet.data()[6],
           uart_packet.data()[7]);
  uart_write_bytes(config::stm_uart::port,
                   uart_packet.data(),
                   uart_packet.size());
  uart_packet = LapplPacket::make_read_packet(LapplAddress::RIGHT_TORQUE)
                    .get_raw_packet();
  ESP_LOGI("UART",
           "%02x %02x %02x %02x %02x %02x %02x %02x",
           uart_packet.data()[0],
           uart_packet.data()[1],
           uart_packet.data()[2],
           uart_packet.data()[3],
           uart_packet.data()[4],
           uart_packet.data()[5],
           uart_packet.data()[6],
           uart_packet.data()[7]);
  uart_write_bytes(config::stm_uart::port,
                   uart_packet.data(),
                   uart_packet.size());
  uart_packet =
      LapplPacket::make_read_packet(LapplAddress::LEFT_TORQUE).get_raw_packet();
  ESP_LOGI("UART",
           "%02x %02x %02x %02x %02x %02x %02x %02x",
           uart_packet.data()[0],
           uart_packet.data()[1],
           uart_packet.data()[2],
           uart_packet.data()[3],
           uart_packet.data()[4],
           uart_packet.data()[5],
           uart_packet.data()[6],
           uart_packet.data()[7]);
  uart_write_bytes(config::stm_uart::port,
                   uart_packet.data(),
                   uart_packet.size());
  uart_packet = LapplPacket::make_read_packet(LapplAddress::CONTROL_MODE)
                    .get_raw_packet();
  ESP_LOGI("UART",
           "%02x %02x %02x %02x %02x %02x %02x %02x",
           uart_packet.data()[0],
           uart_packet.data()[1],
           uart_packet.data()[2],
           uart_packet.data()[3],
           uart_packet.data()[4],
           uart_packet.data()[5],
           uart_packet.data()[6],
           uart_packet.data()[7]);
  uart_write_bytes(config::stm_uart::port,
                   uart_packet.data(),
                   uart_packet.size());

  auto root = cJSON_CreateObject();

  for (int i = 0; i < 4; i++) {
    auto response = context::http_service.queue.receive(50);
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
  auto uart_packet = LapplPacket::make_write_packet(LapplAddress::RUNNING, true)
                         .get_raw_packet();
  uart_write_bytes(config::stm_uart::port,
                   uart_packet.data(),
                   uart_packet.size());
  ESP_RETURN_ON_ERROR(httpd_resp_send(req, nullptr, 0),
                      HttpService::LOG_TAG,
                      "Start response transmission failed");
  return ESP_OK;
}

esp_err_t HttpService::stop_handler(httpd_req_t* req) {
  HttpService::allow_cors(req);
  auto uart_packet =
      LapplPacket::make_write_packet(LapplAddress::RUNNING, false)
          .get_raw_packet();
  uart_write_bytes(config::stm_uart::port,
                   uart_packet.data(),
                   uart_packet.size());
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
    auto uart_packet =
        LapplPacket::make_write_packet(LapplAddress::RIGHT_TORQUE, torque)
            .get_raw_packet();
    uart_write_bytes(config::stm_uart::port,
                     uart_packet.data(),
                     uart_packet.size());
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
    auto uart_packet =
        LapplPacket::make_write_packet(LapplAddress::LEFT_TORQUE, torque)
            .get_raw_packet();
    uart_write_bytes(config::stm_uart::port,
                     uart_packet.data(),
                     uart_packet.size());
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
  auto uart_packet =
      LapplPacket::make_write_packet(LapplAddress::CONTROL_MODE,
                                     static_cast<uint8_t>(ControlMode::MANUAL))
          .get_raw_packet();
  uart_write_bytes(config::stm_uart::port,
                   uart_packet.data(),
                   uart_packet.size());
  ESP_RETURN_ON_ERROR(httpd_resp_send(req, nullptr, 0),
                      HttpService::LOG_TAG,
                      "Stop response transmission failed");
  return ESP_OK;
}
esp_err_t HttpService::set_mode_automatic_handler(httpd_req_t* req) {
  HttpService::allow_cors(req);
  auto uart_packet =
      LapplPacket::make_write_packet(LapplAddress::CONTROL_MODE,
                                     static_cast<uint8_t>(ControlMode::AUTO))
          .get_raw_packet();
  uart_write_bytes(config::stm_uart::port,
                   uart_packet.data(),
                   uart_packet.size());
  ESP_RETURN_ON_ERROR(httpd_resp_send(req, nullptr, 0),
                      HttpService::LOG_TAG,
                      "Stop response transmission failed");
  return ESP_OK;
}
esp_err_t HttpService::set_mode_semi_automatic_handler(httpd_req_t* req) {
  HttpService::allow_cors(req);
  auto uart_packet = LapplPacket::make_write_packet(
                         LapplAddress::CONTROL_MODE,
                         static_cast<uint8_t>(ControlMode::SEMI_AUTO))
                         .get_raw_packet();
  uart_write_bytes(config::stm_uart::port,
                   uart_packet.data(),
                   uart_packet.size());
  ESP_RETURN_ON_ERROR(httpd_resp_send(req, nullptr, 0),
                      HttpService::LOG_TAG,
                      "Stop response transmission failed");
  return ESP_OK;
}
esp_err_t HttpService::set_mode_smart_handler(httpd_req_t* req) {
  HttpService::allow_cors(req);
  auto uart_packet =
      LapplPacket::make_write_packet(LapplAddress::CONTROL_MODE,
                                     static_cast<uint8_t>(ControlMode::SMART))
          .get_raw_packet();
  uart_write_bytes(config::stm_uart::port,
                   uart_packet.data(),
                   uart_packet.size());

  ESP_RETURN_ON_ERROR(httpd_resp_send(req, nullptr, 0),
                      HttpService::LOG_TAG,
                      "Stop response transmission failed");
  return ESP_OK;
}

esp_err_t HttpService::options_handler(httpd_req_t* req) {
  HttpService::allow_cors(req);
  httpd_resp_send(req, NULL, 0);
  return ESP_OK;
}

esp_err_t HttpService::register_uri(const char* uri_address,
                                    httpd_method_t method,
                                    esp_err_t (*handler)(httpd_req_t* req)) {
  httpd_uri uri = {
      .uri = uri_address,
      .method = method,
      .handler = handler,
      .user_ctx = nullptr,
  };
  ESP_RETURN_ON_ERROR(httpd_register_uri_handler(this->server_instance, &uri),
                      HttpService::LOG_TAG,
                      "Failed to register %s end point",
                      uri_address);
  return ESP_OK;
}

esp_err_t HttpService::register_uri_with_option(
    const char* uri_address,
    httpd_method_t method,
    esp_err_t (*handler)(httpd_req_t* req)) {
  httpd_uri uri = {
      .uri = uri_address,
      .method = method,
      .handler = handler,
      .user_ctx = nullptr,
  };

  httpd_uri option_uri = {
      .uri = uri_address,
      .method = HTTP_OPTIONS,
      .handler = HttpService::options_handler,
      .user_ctx = nullptr,
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

esp_err_t HttpService::register_dynamic_endpoints() {
  this->register_uri("/api/states", HTTP_GET, HttpService::get_state_handler);
  this->register_uri_with_option("/api/start",
                                 HTTP_PUT,
                                 HttpService::start_handler);
  this->register_uri_with_option("/api/stop",
                                 HTTP_PUT,
                                 HttpService::stop_handler);
  this->register_uri_with_option("/api/right_torque",
                                 HTTP_PUT,
                                 HttpService::set_right_torque_handler);
  this->register_uri_with_option("/api/left_torque",
                                 HTTP_PUT,
                                 HttpService::set_left_torque_handler);
  this->register_uri_with_option("/api/set-mode/manual",
                                 HTTP_PUT,
                                 HttpService::set_mode_manual_handler);
  this->register_uri_with_option("/api/set-mode/automatic",
                                 HTTP_PUT,
                                 HttpService::set_mode_automatic_handler);
  this->register_uri_with_option("/api/set-mode/semi-automatic",
                                 HTTP_PUT,
                                 HttpService::set_mode_semi_automatic_handler);
  this->register_uri_with_option("/api/set-mode/smart",
                                 HTTP_PUT,
                                 HttpService::set_mode_smart_handler);
  return ESP_OK;
}

void HttpService::start() {
  httpd_config_t http_config = HTTPD_DEFAULT_CONFIG();
  http_config.max_uri_handlers = 32;
  ESP_ERROR_CHECK(httpd_start(&server_instance, &http_config));
  http_server_register_assets(server_instance);
  ESP_ERROR_CHECK(register_dynamic_endpoints());
}
