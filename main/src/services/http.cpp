#include <cmath>
#include <cstdio>

#include "services/http.hpp"

#include "esp_check.h"
#include "esp_err.h"
#include "esp_http_server.h"
#include "http_assets.hpp"
#include "http_parser.h"
#include "portmacro.h"
#include "report.hpp"

#include <cJSON.h>
#include "context.hpp"
#include "services/stm_uart/packet.hpp"
#include "services/stm_uart_tx.hpp"

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

esp_err_t HttpService::get_state_handler(httpd_req_t* req) {
  auto uart_packet = UartPacket::make_get_running_packet();
  context::stm_uart_tx_service.queue.send(uart_packet, portMAX_DELAY);
  uart_packet = UartPacket::make_get_right_manual_packet();
  context::stm_uart_tx_service.queue.send(uart_packet, portMAX_DELAY);
  uart_packet = UartPacket::make_get_left_manual_packet();
  context::stm_uart_tx_service.queue.send(uart_packet, portMAX_DELAY);
  uart_packet = UartPacket::make_get_mode_packet();
  context::stm_uart_tx_service.queue.send(uart_packet, portMAX_DELAY);

  auto running_response = context::http_service.queue.receive(50);
  auto right_manual_torque_response = context::http_service.queue.receive(50);
  auto left_manual_torque_response = context::http_service.queue.receive(50);
  auto mode_response = context::http_service.queue.receive(50);

  if (!running_response.has_value() ||
      !right_manual_torque_response.has_value() ||
      !left_manual_torque_response.has_value() || !mode_response.has_value()) {
    httpd_resp_send_err(req,
                        HTTPD_500_INTERNAL_SERVER_ERROR,
                        "STM32 not responding");
    return ESP_OK;
  }
  auto root = cJSON_CreateObject();
  cJSON_AddBoolToObject(root, "running", running_response->payload.b);
  cJSON_AddNumberToObject(root,
                          "rightTorque",
                          right_manual_torque_response->payload.f);
  cJSON_AddNumberToObject(root,
                          "leftTorque",
                          left_manual_torque_response->payload.f);
  cJSON_AddStringToObject(
      root,
      "mode",
      get_contorl_mode_str(mode_response->payload.control_mode));

  auto json_str = cJSON_PrintUnformatted(root);
  httpd_resp_set_type(req, "application/json");
  ESP_RETURN_ON_ERROR(httpd_resp_send(req, json_str, HTTPD_RESP_USE_STRLEN),
                      HttpService::LOG_TAG,
                      "Get running response transmission failed");
  return ESP_OK;
}

// esp_err_t HttpService::get_right_torque(httpd_req_t* req) {

// }

esp_err_t HttpService::start_handler(httpd_req_t* req) {
  auto uart_packet = UartPacket::make_start_packet();
  context::stm_uart_tx_service.queue.send(uart_packet, portMAX_DELAY);
  ESP_RETURN_ON_ERROR(httpd_resp_send(req, nullptr, 0),
                      HttpService::LOG_TAG,
                      "Start response transmission failed");
  return ESP_OK;
}

esp_err_t HttpService::stop_handler(httpd_req_t* req) {
  auto uart_packet = UartPacket::make_stop_packet();
  context::stm_uart_tx_service.queue.send(uart_packet, portMAX_DELAY);
  ESP_RETURN_ON_ERROR(httpd_resp_send(req, nullptr, 0),
                      HttpService::LOG_TAG,
                      "Stop response transmission failed");
  return ESP_OK;
}
esp_err_t HttpService::set_right_torque_handler(httpd_req_t* req) {
  int len = req->content_len;
  char* body = (char*)malloc(len + 1);

  int received = httpd_req_recv(req, body, len);
  body[received] = 0;

  cJSON* root = cJSON_Parse(body);
  cJSON* torqueItem = cJSON_GetObjectItem(root, "torque");

  if (cJSON_IsNumber(torqueItem)) {
    int torque = torqueItem->valueint;
    auto uart_packet = UartPacket::make_set_right_torque_packet(torque);
    context::stm_uart_tx_service.queue.send(uart_packet, portMAX_DELAY);
  }

  cJSON_Delete(root);
  free(body);

  ESP_RETURN_ON_ERROR(httpd_resp_send(req, nullptr, 0),
                      HttpService::LOG_TAG,
                      "Stop response transmission failed");
  return ESP_OK;
}

esp_err_t HttpService::set_left_torque_handler(httpd_req_t* req) {
  int len = req->content_len;
  char* body = (char*)malloc(len + 1);

  int received = httpd_req_recv(req, body, len);
  body[received] = 0;

  cJSON* root = cJSON_Parse(body);
  cJSON* torqueItem = cJSON_GetObjectItem(root, "torque");

  if (cJSON_IsNumber(torqueItem)) {
    int torque = torqueItem->valueint;
    auto uart_packet = UartPacket::make_set_left_torque_packet(torque);
    context::stm_uart_tx_service.queue.send(uart_packet, portMAX_DELAY);
  }

  cJSON_Delete(root);
  free(body);

  ESP_RETURN_ON_ERROR(httpd_resp_send(req, nullptr, 0),
                      HttpService::LOG_TAG,
                      "Stop response transmission failed");

  return ESP_OK;
}

esp_err_t HttpService::set_mode_manual_handler(httpd_req_t* req) {
  auto uart_packet = UartPacket::make_set_mode_packet(ControlMode::MANUAL);
  context::stm_uart_tx_service.queue.send(uart_packet, portMAX_DELAY);
  ESP_RETURN_ON_ERROR(httpd_resp_send(req, nullptr, 0),
                      HttpService::LOG_TAG,
                      "Stop response transmission failed");
  return ESP_OK;
}
esp_err_t HttpService::set_mode_automatic_handler(httpd_req_t* req) {
  auto uart_packet = UartPacket::make_set_mode_packet(ControlMode::AUTO);
  context::stm_uart_tx_service.queue.send(uart_packet, portMAX_DELAY);
  ESP_RETURN_ON_ERROR(httpd_resp_send(req, nullptr, 0),
                      HttpService::LOG_TAG,
                      "Stop response transmission failed");
  return ESP_OK;
}
esp_err_t HttpService::set_mode_semi_automatic_handler(httpd_req_t* req) {
  auto uart_packet = UartPacket::make_set_mode_packet(ControlMode::SEMI_AUTO);
  context::stm_uart_tx_service.queue.send(uart_packet, portMAX_DELAY);
  ESP_RETURN_ON_ERROR(httpd_resp_send(req, nullptr, 0),
                      HttpService::LOG_TAG,
                      "Stop response transmission failed");
  return ESP_OK;
}
esp_err_t HttpService::set_mode_smart_handler(httpd_req_t* req) {
  auto uart_packet = UartPacket::make_set_mode_packet(ControlMode::SMART);
  context::stm_uart_tx_service.queue.send(uart_packet, portMAX_DELAY);

  ESP_RETURN_ON_ERROR(httpd_resp_send(req, nullptr, 0),
                      HttpService::LOG_TAG,
                      "Stop response transmission failed");
  return ESP_OK;
}

esp_err_t HttpService::register_dynamic_endpoints() {
  httpd_uri get_states_uri = {
      .uri = "/api/states",
      .method = HTTP_GET,
      .handler = HttpService::get_state_handler,
      .user_ctx = nullptr,
  };

  httpd_uri start_uri = {
      .uri = "/api/start",
      .method = HTTP_PUT,
      .handler = HttpService::start_handler,
      .user_ctx = nullptr,
  };

  httpd_uri stop_uri = {
      .uri = "/api/stop",
      .method = HTTP_PUT,
      .handler = HttpService::stop_handler,
      .user_ctx = nullptr,
  };

  httpd_uri set_right_torque_uri = {
      .uri = "/api/right_torque",
      .method = HTTP_PUT,
      .handler = HttpService::set_right_torque_handler,
      .user_ctx = nullptr,
  };

  httpd_uri set_left_torque_uri = {
      .uri = "/api/left_torque",
      .method = HTTP_PUT,
      .handler = HttpService::set_left_torque_handler,
      .user_ctx = nullptr,
  };

  httpd_uri set_mode_manual_uri = {
      .uri = "/api/set-mode/manual",
      .method = HTTP_PUT,
      .handler = HttpService::set_mode_manual_handler,
      .user_ctx = nullptr,
  };

  httpd_uri set_mode_automatic_uri = {
      .uri = "/api/set-mode/automatic",
      .method = HTTP_PUT,
      .handler = HttpService::set_mode_automatic_handler,
      .user_ctx = nullptr,
  };

  httpd_uri set_mode_semi_automatic_uri = {
      .uri = "/api/set-mode/semi-automatic",
      .method = HTTP_PUT,
      .handler = HttpService::set_mode_semi_automatic_handler,
      .user_ctx = nullptr,
  };

  httpd_uri set_mode_smart_uri = {
      .uri = "/api/set-mode/smart",
      .method = HTTP_PUT,
      .handler = HttpService::set_mode_smart_handler,
      .user_ctx = nullptr,
  };

  ESP_RETURN_ON_ERROR(
      httpd_register_uri_handler(this->server_instance, &get_states_uri),
      HttpService::LOG_TAG,
      "Failed to register /api/running end point");
  ESP_RETURN_ON_ERROR(
      httpd_register_uri_handler(this->server_instance, &start_uri),
      HttpService::LOG_TAG,
      "Failed to register /api/start end point");
  ESP_RETURN_ON_ERROR(
      httpd_register_uri_handler(this->server_instance, &stop_uri),
      HttpService::LOG_TAG,
      "Failed to register /api/stop end point");
  ESP_RETURN_ON_ERROR(
      httpd_register_uri_handler(this->server_instance, &set_left_torque_uri),
      HttpService::LOG_TAG,
      "Failed to register /api/left_torque end point");
  ESP_RETURN_ON_ERROR(
      httpd_register_uri_handler(this->server_instance, &set_right_torque_uri),
      HttpService::LOG_TAG,
      "Failed to register /api/right_torque end point");
  ESP_RETURN_ON_ERROR(
      httpd_register_uri_handler(this->server_instance, &set_mode_manual_uri),
      HttpService::LOG_TAG,
      "Failed to register /api/set-mode/manual end point");
  ESP_RETURN_ON_ERROR(httpd_register_uri_handler(this->server_instance,
                                                 &set_mode_automatic_uri),
                      HttpService::LOG_TAG,
                      "Failed to register /api/set-mode/automatic end point");
  ESP_RETURN_ON_ERROR(
      httpd_register_uri_handler(this->server_instance,
                                 &set_mode_semi_automatic_uri),
      HttpService::LOG_TAG,
      "Failed to register /api/set-mode/semi-automatic end point");
  ESP_RETURN_ON_ERROR(
      httpd_register_uri_handler(this->server_instance, &set_mode_smart_uri),
      HttpService::LOG_TAG,
      "Failed to register /api/set-mode/smart end point");
  return ESP_OK;
}

void HttpService::start() {
  httpd_config_t http_config = HTTPD_DEFAULT_CONFIG();
  http_config.max_uri_handlers = 12;
  ESP_ERROR_CHECK(httpd_start(&server_instance, &http_config));
  http_server_register_assets(server_instance);
  ESP_ERROR_CHECK(register_dynamic_endpoints());
}
