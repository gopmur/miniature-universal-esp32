#include "http/modules/motor.hpp"
#include "esp_http_server.h"
#include "helper/formats.hpp"
#include "jayson.hpp"
#include "tasks/motor.hpp"
#include "tasks/ws.hpp"

extern MotorTask* motor_task;
extern WebSocketTask ws_task;

esp_err_t HttpMotorModule::get_zero_pos(httpd_req_t* req) {
  motor_task->zero_pos_all();
  return send_success_json(req);
}
esp_err_t HttpMotorModule::ws_notification_status(httpd_req_t* req) {
  return ESP_OK;
}
esp_err_t HttpMotorModule::ws_notification_status_post_handshake(httpd_req_t* req) {
  auto fd = httpd_req_to_sockfd(req);
  ws_task.add_connection(fd, WebSocketConnectionType::MOTOR_STATUS);
  return ESP_OK;
}
esp_err_t HttpMotorModule::get_status(httpd_req_t* req) {
  JsonArray resp_array;
  for (size_t i = 0; i < motor_task->motor_count; i++) {
    auto status = motor_task->get_status(i);
    auto can_id = motor_task->get_can_id(i);
    JsonObject json;
    json.set("canId", can_id);
    json.set("status", get_motor_status_string(status));
    resp_array.append_object(&json);
  }
  send_json(req, resp_array);
  return ESP_OK;
}

void HttpMotorModule::register_direct_uris() {
  register_uri("/zero-pos", HTTP_GET, get_zero_pos);
  register_uri("/status", HTTP_GET, get_status);
  register_ws_uri("/notification/status",
                  ws_notification_status,
                  ws_notification_status_post_handshake);
}
