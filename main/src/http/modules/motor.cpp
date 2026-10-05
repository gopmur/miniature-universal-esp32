#include "http/modules/motor.hpp"
#include "esp_http_server.h"
#include "tasks/motor.hpp"
#include "tasks/ws.hpp"

extern MotorTask* motor_task;
extern WebSocketTask ws_task;

esp_err_t HttpMotorModule::get_zero_pos(httpd_req_t* req) {
  motor_task->zero_pos_all();
  return send_success_json(req);
}
esp_err_t HttpMotorModule::ws_status(httpd_req_t* req) {
  return ESP_OK;
}
esp_err_t HttpMotorModule::ws_status_post_handshake(httpd_req_t* req) {
  auto fd = httpd_req_to_sockfd(req);
  ws_task.add_connection(fd, WebSocketConnectionType::MOTOR_STATUS);
  return ESP_OK;
}

void HttpMotorModule::register_direct_uris() {
  register_uri("/zero-pos", HTTP_GET, get_zero_pos);
  register_ws_uri("/status", ws_status, ws_status_post_handshake);
}
