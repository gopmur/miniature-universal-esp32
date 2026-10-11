#include "tasks/control.hpp"
#include <cstring>
#include <variant>
#include "esp_err.h"
#include "esp_http_server.h"
#include "http/modules/control.hpp"
#include "http_parser.h"
#include "jayson.hpp"
#include "system_logger.hpp"
#include "tasks/motor.hpp"

extern ControlTask* control_task;
extern MotorTask* motor_task;

const char* HttpControlModule::get_control_mode_str(ControlMode control_mode) {
  switch (control_mode) {
    case ControlMode::HEXA_MANUAL:
      return "manual";
    case ControlMode::HEXA_AUTOMATIC:
      return "automatic";
    case ControlMode::HEXA_SEMIAUTOMATIC:
      return "semi-automatic";
    case ControlMode::HEXA_SMART:
      return "smart";
  }
  return "undefined";
}

const char* HttpControlModule::get_leg_str(Leg leg) {
  switch (leg) {
    case Leg::LEFT:
      return "left";
    case Leg::RIGHT:
      return "right";
  }
  return "undefined";
}

esp_err_t HttpControlModule::get_params(httpd_req_t* req) {
  JsonObject res_json;
  JsonObject control_params_json;
  JsonObject manual_control_params_json;
  JsonObject automatic_control_params_json;
  JsonObject semiautomatic_control_params_json;
  JsonObject smart_control_params_json;
  JsonObject manual_control_params_left_json;
  JsonObject automatic_control_params_left_json;
  JsonObject semiautomatic_control_params_left_json;
  JsonObject smart_control_params_left_json;
  JsonObject manual_control_params_right_json;
  JsonObject automatic_control_params_right_json;
  JsonObject semiautomatic_control_params_right_json;
  JsonObject smart_control_params_right_json;

  res_json.set("running", control_task->running);
  res_json.set("mode", get_control_mode_str(control_task->control_mode));
  manual_control_params_left_json.set("torque",
                                      control_task->hexa_controller.manual.params.left.torque);
  manual_control_params_right_json.set("torque",
                                       control_task->hexa_controller.manual.params.right.torque);
  automatic_control_params_left_json.set(
      "torque",
      control_task->hexa_controller.automatic.params.left.torque);
  automatic_control_params_left_json.set(
      "timeout",
      control_task->hexa_controller.automatic.params.left.timeout);
  automatic_control_params_left_json.set(
      "velocityThreshold",
      control_task->hexa_controller.automatic.configs.get_left_velocity_threshold());
  automatic_control_params_right_json.set(
      "torque",
      control_task->hexa_controller.automatic.params.right.torque);
  automatic_control_params_right_json.set(
      "timeout",
      control_task->hexa_controller.automatic.params.right.timeout);
  automatic_control_params_right_json.set(
      "velocityThreshold",
      control_task->hexa_controller.automatic.configs.get_right_velocity_threshold());
  semiautomatic_control_params_json.set(
      "weakLeg",
      get_leg_str(control_task->hexa_controller.semiautomatic.params.weak_leg));
  semiautomatic_control_params_left_json.set(
      "torque",
      control_task->hexa_controller.semiautomatic.params.left.torque);
  semiautomatic_control_params_left_json.set(
      "timeout",
      control_task->hexa_controller.semiautomatic.params.left.timeout);
  semiautomatic_control_params_left_json.set(
      "delay",
      control_task->hexa_controller.semiautomatic.params.left.delay);
  semiautomatic_control_params_right_json.set(
      "torque",
      control_task->hexa_controller.semiautomatic.params.right.torque);
  semiautomatic_control_params_right_json.set(
      "timeout",
      control_task->hexa_controller.semiautomatic.params.right.timeout);
  semiautomatic_control_params_right_json.set(
      "delay",
      control_task->hexa_controller.semiautomatic.params.right.delay);
  semiautomatic_control_params_json.set(
      "startAssistAngle",
      control_task->hexa_controller.semiautomatic.configs.get_start_assist_angle());
  semiautomatic_control_params_json.set(
      "stopAssistAngle",
      control_task->hexa_controller.semiautomatic.configs.get_stop_assist_angle());
  semiautomatic_control_params_json.set(
      "epsilon",
      control_task->hexa_controller.semiautomatic.configs.get_epsilon());
  smart_control_params_left_json.set("torque",
                                     control_task->hexa_controller.smart.params.left.torque);
  smart_control_params_right_json.set("torque",
                                      control_task->hexa_controller.smart.params.right.torque);

  manual_control_params_json.set("right", &manual_control_params_right_json);
  manual_control_params_json.set("left", &manual_control_params_left_json);
  automatic_control_params_json.set("right", &automatic_control_params_right_json);
  automatic_control_params_json.set("left", &automatic_control_params_left_json);
  semiautomatic_control_params_json.set("right", &semiautomatic_control_params_right_json);
  semiautomatic_control_params_json.set("left", &semiautomatic_control_params_left_json);
  smart_control_params_json.set("right", &smart_control_params_right_json);
  smart_control_params_json.set("left", &smart_control_params_left_json);
  control_params_json.set("manual", &manual_control_params_json);
  control_params_json.set("automatic", &automatic_control_params_json);
  control_params_json.set("semiautomatic", &semiautomatic_control_params_json);
  control_params_json.set("smart", &smart_control_params_json);
  res_json.set("controlParams", &control_params_json);

  auto json_str = res_json.stringify();
  return send_json(req, res_json);
}

esp_err_t HttpControlModule::put_start(httpd_req_t* req) {
  control_task->running = true;
  motor_task->enable_all();
  return send_success_json(req);
}

esp_err_t HttpControlModule::put_stop(httpd_req_t* req) {
  control_task->running = false;
  motor_task->disable_all();
  return send_success_json(req);
}

esp_err_t HttpControlModule::put_set_mode_manual(httpd_req_t* req) {
  control_task->control_mode = ControlMode::HEXA_MANUAL;
  return send_success_json(req);
}

esp_err_t HttpControlModule::put_set_mode_automatic(httpd_req_t* req) {
  control_task->control_mode = ControlMode::HEXA_AUTOMATIC;
  return send_success_json(req);
}

esp_err_t HttpControlModule::put_set_mode_semiautomatic(httpd_req_t* req) {
  control_task->control_mode = ControlMode::HEXA_SEMIAUTOMATIC;
  return send_success_json(req);
}

esp_err_t HttpControlModule::put_set_mode_smart(httpd_req_t* req) {
  control_task->control_mode = ControlMode::HEXA_SMART;
  return send_success_json(req);
}

esp_err_t HttpControlModule::ws_manual_torque_post_handshake(httpd_req_t* req) {
  return ESP_OK;
}

esp_err_t HttpControlModule::ws_manual_torque(httpd_req_t* req) {
  httpd_ws_frame_t ws_frame;
  memset(&ws_frame, 0, sizeof(ws_frame));
  ws_frame.type = HTTPD_WS_TYPE_TEXT;
  httpd_ws_recv_frame(req, &ws_frame, 0);
  if (!ws_frame.len) {
    return ESP_OK;
  }
  ws_frame.payload = static_cast<uint8_t*>(malloc(ws_frame.len + 1));
  httpd_ws_recv_frame(req, &ws_frame, ws_frame.len);
  ws_frame.payload[ws_frame.len] = 0;
  auto data_result = JsonObject::parse(reinterpret_cast<char*>(ws_frame.payload));

  if (std::holds_alternative<JsonError>(data_result)) {
    free(ws_frame.payload);
    return ESP_OK;
  }

  auto data = std::get<JsonObject>(data_result);

  auto left_torque = data.get_number("leftTorque");
  auto right_torque = data.get_number("rightTorque");
  if (std::holds_alternative<double>(left_torque)) {
    control_task->hexa_controller.manual.params.left.torque = std::get<double>(left_torque);
  }
  if (std::holds_alternative<double>(right_torque)) {
    control_task->hexa_controller.manual.params.right.torque = std::get<double>(right_torque);
  }

  free(ws_frame.payload);
  return ESP_OK;
}

esp_err_t HttpControlModule::put_manual_params(httpd_req_t* req, JsonObject* req_json) {
  JsonObject resp_json;

  auto left_json_result = req_json->get_object("left", &resp_json);
  auto right_json_result = req_json->get_object("right", &resp_json);

  if (std::holds_alternative<JsonError>(left_json_result) ||
      std::holds_alternative<JsonError>(right_json_result)) {
    return send_json(req, resp_json, HTTPD_400_BAD_REQUEST);
  }

  auto left_json = std::get<JsonObject>(left_json_result);
  auto right_json = std::get<JsonObject>(right_json_result);

  auto left_torque_result = left_json.get_number("torque", &resp_json);
  auto right_torque_result = right_json.get_number("torque", &resp_json);

  if (std::holds_alternative<JsonError>(left_torque_result) ||
      std::holds_alternative<JsonError>(right_torque_result)) {
    return send_json(req, resp_json, HTTPD_400_BAD_REQUEST);
  }
  auto left_torque = std::get<double>(left_torque_result);
  auto right_torque = std::get<double>(right_torque_result);
  LOGI("%d %d", left_torque, right_torque);
  control_task->hexa_controller.manual.params.left.torque = left_torque;
  control_task->hexa_controller.manual.params.right.torque = right_torque;
  return send_success_json(req);
}

esp_err_t HttpControlModule::put_automatic_params(httpd_req_t* req, JsonObject* req_json) {
  JsonObject resp_json;

  auto left_json_result = req_json->get_object("left", &resp_json);
  auto right_json_result = req_json->get_object("right", &resp_json);

  if (std::holds_alternative<JsonError>(left_json_result) ||
      std::holds_alternative<JsonError>(right_json_result)) {
    return send_json(req, resp_json, HTTPD_400_BAD_REQUEST);
  }

  auto left_json = std::get<JsonObject>(left_json_result);
  auto right_json = std::get<JsonObject>(right_json_result);

  auto left_torque_result = left_json.get_number("torque", &resp_json);
  auto right_torque_result = right_json.get_number("torque", &resp_json);
  auto left_timeout_result = left_json.get_number("timeout", &resp_json);
  auto right_timeout_result = right_json.get_number("timeout", &resp_json);

  if (std::holds_alternative<JsonError>(left_torque_result) ||
      std::holds_alternative<JsonError>(right_torque_result) ||
      std::holds_alternative<JsonError>(left_timeout_result) ||
      std::holds_alternative<JsonError>(right_timeout_result)) {
    return send_json(req, resp_json, HTTPD_400_BAD_REQUEST);
  }

  auto left_timeout = std::get<double>(left_timeout_result);
  auto right_timeout = std::get<double>(right_timeout_result);
  auto left_torque = std::get<double>(left_torque_result);
  auto right_torque = std::get<double>(right_torque_result);

  control_task->hexa_controller.automatic.params.right.timeout = right_timeout;
  control_task->hexa_controller.automatic.params.left.timeout = left_timeout;
  control_task->hexa_controller.automatic.params.left.torque = left_torque;
  control_task->hexa_controller.automatic.params.right.torque = right_torque;

  return send_success_json(req);
}

esp_err_t HttpControlModule::put_automatic_configs(httpd_req_t* req, JsonObject* req_json) {
  JsonObject resp_json;
  auto left_json_result = req_json->get_object("left", &resp_json);
  auto right_json_result = req_json->get_object("right", &resp_json);
  auto angle_threshold_result = req_json->get_number("angleThreshold", &resp_json);
  if (std::holds_alternative<JsonError>(left_json_result) ||
      std::holds_alternative<JsonError>(right_json_result) ||
      std::holds_alternative<JsonError>(angle_threshold_result)) {
    return send_json(req, resp_json, HTTPD_400_BAD_REQUEST);
  }

  auto left_json = std::get<JsonObject>(left_json_result);
  auto right_json = std::get<JsonObject>(right_json_result);
  auto angle_threshold = std::get<double>(angle_threshold_result);

  auto left_velocity_threshold_result = left_json.get_number("velocityThreshold", &resp_json);
  auto right_velocity_threshold_result = right_json.get_number("velocityThreshold", &resp_json);

  if (std::holds_alternative<JsonError>(left_velocity_threshold_result) ||
      std::holds_alternative<JsonError>(right_velocity_threshold_result)) {
    return send_json(req, resp_json, HTTPD_400_BAD_REQUEST);
  }

  auto left_velocity_threshold = std::get<double>(left_velocity_threshold_result);
  auto right_velocity_threshold = std::get<double>(right_velocity_threshold_result);

  control_task->hexa_controller.automatic.configs.set_left_velocity_threshold(
      left_velocity_threshold);
  control_task->hexa_controller.automatic.configs.set_right_velocity_threshold(
      right_velocity_threshold);
  control_task->hexa_controller.automatic.configs.set_angle_threshold(angle_threshold);

  return send_success_json(req);
}

esp_err_t HttpControlModule::put_semiautomatic_params(httpd_req_t* req, JsonObject* req_json) {
  JsonObject resp_json;

  auto weak_leg_result = req_json->get_string("weakLeg", &resp_json);
  auto left_json_result = req_json->get_object("left", &resp_json);
  auto right_json_result = req_json->get_object("right", &resp_json);

  if (std::holds_alternative<JsonError>(weak_leg_result) ||
      std::holds_alternative<JsonError>(left_json_result) ||
      std::holds_alternative<JsonError>(left_json_result)) {
    return send_json(req, resp_json, HTTPD_400_BAD_REQUEST);
  }

  auto weak_leg =
      std::strcmp(std::get<char*>(weak_leg_result), "left") == 0 ? Leg::LEFT : Leg::RIGHT;
  auto left_json = std::get<JsonObject>(left_json_result);
  auto right_json = std::get<JsonObject>(right_json_result);
  auto left_torque_result = left_json.get_number("torque", &resp_json);
  auto left_timeout_result = left_json.get_number("timeout", &resp_json);
  auto left_delay_result = left_json.get_number("delay", &resp_json);
  auto right_torque_result = right_json.get_number("torque", &resp_json);
  auto right_timeout_result = right_json.get_number("timeout", &resp_json);
  auto right_delay_result = right_json.get_number("delay", &resp_json);

  if (std::holds_alternative<JsonError>(left_torque_result) ||
      std::holds_alternative<JsonError>(left_delay_result) ||
      std::holds_alternative<JsonError>(left_timeout_result) ||
      std::holds_alternative<JsonError>(right_torque_result) ||
      std::holds_alternative<JsonError>(right_delay_result) ||
      std::holds_alternative<JsonError>(right_timeout_result)) {
    return send_json(req, resp_json, HTTPD_400_BAD_REQUEST);
  }

  auto left_torque = std::get<double>(left_torque_result);
  auto left_timeout = std::get<double>(left_timeout_result);
  auto left_delay = std::get<double>(left_delay_result);
  auto right_torque = std::get<double>(right_torque_result);
  auto right_timeout = std::get<double>(right_timeout_result);
  auto right_delay = std::get<double>(right_delay_result);

  control_task->hexa_controller.semiautomatic.params.left.torque = left_torque;
  control_task->hexa_controller.semiautomatic.params.left.timeout = left_timeout;
  control_task->hexa_controller.semiautomatic.params.left.delay = left_delay;
  control_task->hexa_controller.semiautomatic.params.right.torque = right_torque;
  control_task->hexa_controller.semiautomatic.params.right.timeout = right_timeout;
  control_task->hexa_controller.semiautomatic.params.right.delay = right_delay;
  control_task->hexa_controller.semiautomatic.params.weak_leg = weak_leg;

  return send_success_json(req);
}

esp_err_t HttpControlModule::put_semiautomatic_configs(httpd_req_t* req, JsonObject* req_json) {
  JsonObject resp_json;

  auto start_assist_angle_result = req_json->get_number("startAssistAngle", &resp_json);
  auto stop_assist_angle_result = req_json->get_number("stopAssistAngle", &resp_json);
  auto epsilon_result = req_json->get_number("epsilon", &resp_json);

  if (std::holds_alternative<JsonError>(start_assist_angle_result) ||
      std::holds_alternative<JsonError>(stop_assist_angle_result) ||
      std::holds_alternative<JsonError>(epsilon_result)) {
    return send_json(req, resp_json, HTTPD_400_BAD_REQUEST);
  }

  auto start_assist_angle = std::get<double>(start_assist_angle_result);
  auto stop_assist_angle = std::get<double>(stop_assist_angle_result);
  auto epsilon = std::get<double>(epsilon_result);

  control_task->hexa_controller.semiautomatic.configs.set_start_assist_angle(start_assist_angle);
  control_task->hexa_controller.semiautomatic.configs.set_stop_assist_angle(stop_assist_angle);
  control_task->hexa_controller.semiautomatic.configs.set_epsilon(epsilon);
  return send_success_json(req);
}

esp_err_t HttpControlModule::put_smart_params(httpd_req_t* req, JsonObject* req_json) {
  JsonObject resp_json;

  auto left_json_result = req_json->get_object("left", &resp_json);
  auto right_json_result = req_json->get_object("right", &resp_json);

  if (std::holds_alternative<JsonError>(left_json_result) ||
      std::holds_alternative<JsonError>(right_json_result)) {
    return send_json(req, resp_json, HTTPD_400_BAD_REQUEST);
  }

  auto left_json = std::get<JsonObject>(left_json_result);
  auto right_json = std::get<JsonObject>(right_json_result);

  auto left_torque_result = left_json.get_number("torque", &resp_json);
  auto right_torque_result = right_json.get_number("torque", &resp_json);

  if (std::holds_alternative<JsonError>(left_torque_result) ||
      std::holds_alternative<JsonError>(right_torque_result)) {
    return send_json(req, resp_json, HTTPD_400_BAD_REQUEST);
  }
  auto left_torque = std::get<double>(left_torque_result);
  auto right_torque = std::get<double>(right_torque_result);
  control_task->hexa_controller.smart.params.left.torque = left_torque;
  control_task->hexa_controller.smart.params.right.torque = right_torque;
  return send_success_json(req);
}

esp_err_t HttpControlModule::put_smart_configs(httpd_req_t* req, JsonObject* req_json) {
  JsonObject resp_json;
  auto velocity_threshold_result = req_json->get_number("velocityThreshold", &resp_json);
  if (std::holds_alternative<JsonError>(velocity_threshold_result)) {
    return send_json(req, resp_json, HTTPD_400_BAD_REQUEST);
  }
  auto velocity_threshold = std::get<double>(velocity_threshold_result);
  control_task->hexa_controller.smart.configs.set_velocity_threshold(velocity_threshold);
  return send_success_json(req);
}

void HttpControlModule::register_direct_uris() {
  register_uri("/params", HTTP_GET, get_params);
  register_uri_with_option("/start", HTTP_PUT, put_start);
  register_uri_with_option("/stop", HTTP_PUT, put_stop);
  register_uri_with_option("/set-mode/manual", HTTP_PUT, put_set_mode_manual);
  register_uri_with_option("/set-mode/automatic", HTTP_PUT, put_set_mode_automatic);
  register_uri_with_option("/set-mode/semiautomatic", HTTP_PUT, put_set_mode_semiautomatic);
  register_uri_with_option("/set-mode/smart", HTTP_PUT, put_set_mode_smart);
  register_uri_with_option("/manual/params", HTTP_PUT, put_manual_params);
  register_uri_with_option("/automatic/params", HTTP_PUT, put_automatic_params);
  register_uri_with_option("/semiautomatic/params", HTTP_PUT, put_semiautomatic_params);
  register_uri_with_option("/smart/params", HTTP_PUT, put_smart_params);
  register_uri_with_option("/automatic/configs", HTTP_PUT, put_automatic_configs);
  register_uri_with_option("/semiautomatic/configs", HTTP_PUT, put_automatic_configs);
  register_uri_with_option("/smart/configs", HTTP_PUT, put_smart_configs);
  register_ws_uri("/manual/torque", ws_manual_torque, ws_manual_torque_post_handshake);
}

HttpControlModule::HttpControlModule(const char* name, std::vector<HttpModule*> modules)
    : HttpModule(name, modules) {};
HttpControlModule::HttpControlModule(const char* name) : HttpModule(name) {};
