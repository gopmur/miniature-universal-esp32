#include <cmath>

#include "services/http.hpp"

#include "esp_check.h"
#include "esp_err.h"
#include "esp_http_server.h"
#include "http_assets.hpp"
#include "report.hpp"

esp_err_t HTTPService::get_session_reports_handler(httpd_req_t* req) {
  constexpr int record_count = 8;
  constexpr int data_count = 2000;
  constexpr float sampling_rate = 0.1;

  httpd_resp_set_type(req, "application/octet-stream");
  httpd_resp_set_hdr(req, "Transfer-Encoding", "chunked");

  ReportRecord buffer[record_count];
  ReportRecord record;
  int buffer_index = 0;

  for (int i = 0; i < data_count; i++) {
    record.callback_time += 0.1;
    record.smc_mr_vel = std::sin(record.callback_time);
    record.smc_ml_vel = std::sin(record.callback_time);
    record.smc_mr_pos += record.smc_mr_vel * sampling_rate;
    record.smc_ml_pos += record.smc_ml_vel * sampling_rate;

    buffer[buffer_index] = record;
    buffer_index++;

    if (buffer_index == record_count) {
      buffer_index = 0;
      ESP_RETURN_ON_ERROR(
          httpd_resp_send_chunk(req, reinterpret_cast<char*>(buffer),
                                sizeof(buffer)),
          HTTPService::LOG_TAG, "Chunk transmission failed.");
    }
  };
  if (buffer_index != 0) {
    ESP_RETURN_ON_ERROR(
        httpd_resp_send_chunk(req, reinterpret_cast<char*>(buffer),
                              sizeof(ReportRecord) * buffer_index),
        HTTPService::LOG_TAG, "Chunk transmission failed");
  };
  ESP_RETURN_ON_ERROR(httpd_resp_send_chunk(req, nullptr, 0),
                      HTTPService::LOG_TAG,
                      "Termination of chunk transmission failed");
  return ESP_OK;
}

void HTTPService::start() {
  httpd_config_t http_config = HTTPD_DEFAULT_CONFIG();
  ESP_ERROR_CHECK(httpd_start(&server_instance, &http_config));
  http_server_register_assets(server_instance);
}
