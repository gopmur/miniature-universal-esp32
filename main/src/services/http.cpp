#include <cmath>

#include "services/http.hpp"

#include "esp_check.h"
#include "esp_err.h"
#include "esp_http_server.h"
#include "http_assets.hpp"
#include "http_parser.h"
#include "portmacro.h"
#include "report.hpp"

#include "context.hpp"
#include "services/stm_uart.hpp"

esp_err_t HttpService::get_session_reports_handler(httpd_req_t* req) {
  // if (context::stm_uart_service != nullptr) {
  //   const auto message = HttpToStmUartSignal::REQUEST_REPORT;
  //   xQueueSend(context::http_to_stm_uart_queue, &message, portMAX_DELAY);
  // }

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

esp_err_t HttpService::start_handler(httpd_req_t* req) {
  ESP_RETURN_ON_ERROR(httpd_resp_send(req, nullptr, 0),
                      HttpService::LOG_TAG,
                      "Start response transmission failed");
  UartPacket uart_packet;
  uart_packet.type = UartPacketType::START;
  context::stm_uart_service.queue.send(uart_packet, portMAX_DELAY);
  return ESP_OK;
}

esp_err_t HttpService::stop_handler(httpd_req_t* req) {
  ESP_RETURN_ON_ERROR(httpd_resp_send(req, nullptr, 0),
                      HttpService::LOG_TAG,
                      "Stop response transmission failed");
  return ESP_OK;
}
esp_err_t HttpService::set_right_torque_handler(httpd_req_t* req) {
  return ESP_OK;
}
esp_err_t HttpService::set_left_torque_handler(httpd_req_t* req) {
  return ESP_OK;
}

esp_err_t HttpService::register_dynamic_endpoints() {
  httpd_uri get_session_reports_uri = {
      .uri = "/api/report",
      .method = HTTP_GET,
      .handler = HttpService::get_session_reports_handler,
      .user_ctx = nullptr,
  };

  httpd_uri start_uri = {
      .uri = "/api/start",
      .method = HTTP_GET,
      .handler = HttpService::start_handler,
      .user_ctx = nullptr,
  };

  httpd_uri stop_uri = {
      .uri = "/api/stop",
      .method = HTTP_GET,
      .handler = HttpService::stop_handler,
      .user_ctx = nullptr,
  };

  // httpd_uri get_session_reports_uri = {
  //     .uri = "/api/report",
  //     .method = HTTP_GET,
  //     .handler = HttpService::get_session_reports_handler,
  //     .user_ctx = nullptr,
  // };

  // httpd_uri get_session_reports_uri = {
  //     .uri = "/api/report",
  //     .method = HTTP_GET,
  //     .handler = HttpService::get_session_reports_handler,
  //     .user_ctx = nullptr,
  // };
  ESP_RETURN_ON_ERROR(httpd_register_uri_handler(this->server_instance,
                                                 &get_session_reports_uri),
                      HttpService::LOG_TAG,
                      "Failed to register /api/report end point");
  ESP_RETURN_ON_ERROR(
      httpd_register_uri_handler(this->server_instance, &start_uri),
      HttpService::LOG_TAG,
      "Failed to register /api/start end point");
  ESP_RETURN_ON_ERROR(
      httpd_register_uri_handler(this->server_instance, &stop_uri),
      HttpService::LOG_TAG,
      "Failed to register /api/stop end point");
  return ESP_OK;
}

void HttpService::start() {
  httpd_config_t http_config = HTTPD_DEFAULT_CONFIG();
  ESP_ERROR_CHECK(httpd_start(&server_instance, &http_config));
  http_server_register_assets(server_instance);
  ESP_ERROR_CHECK(register_dynamic_endpoints());
}
