#include "http/modules/log.hpp"
#include "http_parser.h"
#include "tasks/logger.hpp"
#include "tasks/ws.hpp"

extern LoggerTask* logger_task;
extern WebSocketTask ws_task;

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

esp_err_t HttpLogModule::ws_system(httpd_req_t* req) {
  return ESP_OK;
}

esp_err_t HttpLogModule::ws_system_post_handshake(httpd_req_t* req) {
  auto client_fd = httpd_req_to_sockfd(req);
  ws_task.add_connection(client_fd, WsStream::SYS_LOG);
  return ESP_OK;
}

void HttpLogModule::register_direct_uris() {
  register_uri("/session/start", HTTP_GET, get_start);
  register_uri("/session/stop", HTTP_GET, get_stop);
  register_ws_uri("/system", ws_system, ws_system_post_handshake);
}