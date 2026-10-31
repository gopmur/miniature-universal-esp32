#include "http/modules/motor.hpp"
#include <variant>
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
  return send_json(req, resp_array);
}

esp_err_t HttpMotorModule::get(httpd_req_t* req) {
  JsonArray resp_array;
  JsonObject json;
  for (size_t i = 0; i < motor_task->motor_count; i++) {
    auto can_id = motor_task->get_can_id(i);
    auto torque_limit = motor_task->get_torque_limit(i);
    auto torque_constant = motor_task->get_torque_constant(i);
    auto direction = motor_task->get_direction(i);
    json.set("canId", can_id);
    json.set("torqueLimit", torque_limit);
    json.set("torqueConstant", torque_constant);
    json.set("direction", get_motor_direction_string(direction));
    resp_array.append_object(&json);
  }
  return send_json(req, resp_array);
};

esp_err_t HttpMotorModule::put_config(httpd_req_t* req, JsonObject* req_json) {
  JsonObject resp_json;
  auto motor_index_result = req_json->get_number("index", &resp_json);
  if (std::holds_alternative<JsonError>(motor_index_result)) {
    return send_json(req, resp_json);
  }
  auto motor_index = static_cast<size_t>(std::get<double>(motor_index_result));
  auto torque_constant_result = req_json->get_number("torqueConstant");
  auto torque_limit_result = req_json->get_number("torqueLimit");
  if (std::holds_alternative<JsonError>(torque_constant_result)) {
    auto torque_constant_error = std::get<JsonError>(torque_constant_result);
    if (torque_constant_error != JsonError::NOT_PROVIDED) {
      req_json->get_number("torqueConstant", &resp_json);
    }
  } else {
    auto torque_constant = std::get<double>(torque_constant_result);
    motor_task->set_torque_constant(motor_index, torque_constant);
  };
  if (std::holds_alternative<JsonError>(torque_limit_result)) {
    auto torque_limit_error = std::get<JsonError>(torque_limit_result);
    if (torque_limit_error != JsonError::NOT_PROVIDED) {
      req_json->get_number("torqueLimit", &resp_json);
    }
  } else {
    auto torque_limit = std::get<double>(torque_limit_result);
    motor_task->set_torque_limit(motor_index, torque_limit);
  }
  if (!resp_json.is_empty()) {
    return send_json(req, resp_json, HTTPD_400_BAD_REQUEST);
  }
  return send_success_json(req);
};

void HttpMotorModule::register_direct_uris() {
  register_uri("/zero-pos", HTTP_GET, get_zero_pos);
  register_uri("/status", HTTP_GET, get_status);
  register_uri("/", HTTP_GET, get);
  register_uri_with_option("/config", HTTP_PUT, put_config);
  register_ws_uri("/notification/status",
                  ws_notification_status,
                  ws_notification_status_post_handshake);
}
