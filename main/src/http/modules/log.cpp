#include "http/modules/log.hpp"
#include "http_parser.h"
#include "tasks/logger.hpp"

extern LoggerTask* logger_task;

esp_err_t HttpLogModule::get_start(httpd_req_t* req) {
  set_header(req);
  logger_task->start_new_log();
  httpd_resp_send(req, nullptr, 0);
  return ESP_OK;
}

esp_err_t HttpLogModule::get_stop(httpd_req_t* req) {
  set_header(req);
  logger_task->stop_log();
  httpd_resp_send(req, nullptr, 0);
  return ESP_OK;
}

void HttpLogModule::register_direct_uris() {
  register_uri("/start", HTTP_GET, get_start);
  register_uri("/stop", HTTP_GET, get_stop);
}