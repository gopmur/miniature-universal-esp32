#include "http/modules/legacy.hpp"
#include <cstdio>
#include <variant>
#include "custom_drivers/motor.hpp"
#include "esp_err.h"
#include "esp_http_server.h"
#include "jayson.hpp"
#include "system_logger.hpp"
#include "tasks/control.hpp"
#include "tasks/motor.hpp"

extern MotorTask* motor_task;
extern AbstractMotorDriver* left_motor;
extern AbstractMotorDriver* right_motor;
extern ControlTask* control_task;

esp_err_t HttpLegacyModule::ws(httpd_req_t* req) {
  LOGI("data received");
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
  printf("%s\n", reinterpret_cast<char*>(ws_frame.payload));
  auto json_result = JsonObject::parse(reinterpret_cast<char*>(ws_frame.payload));
  if (std::holds_alternative<JsonError>(json_result)) {
    auto error = std::get<JsonError>(json_result);
    if (error == JsonError::PARSE_ERROR) {
      LOGE("received message is not a json object. discarding message");
      return ESP_OK;
    } else {
      LOGE("an unhandled error has occurred. message parsing was unsuccessful");
      return ESP_OK;
    }
  }
  auto json = std::get<JsonObject>(json_result);
  auto action_result = json.get_string("action");
  if (std::holds_alternative<JsonError>(action_result)) {
    auto error = std::get<JsonError>(action_result);
    LOGE("json error \"%s\" occurred while reading action", Json::get_json_error_string(error));
    return ESP_OK;
  }
  auto action = std::string(std::get<char*>(action_result));
  if (action == "ping") {
    handle_ping(req);
  } else if (action == "disable") {
    handle_disable(req);
  } else if (action == "enable") {
    handle_enable(req);
  } else if (action == "initialize") {
    handle_initialize(req);
  } else if (action == "assist") {
    handle_assist(req, json);
  } else if (action == "released") {
    handle_release(req);
  }
  return ESP_OK;
}

esp_err_t HttpLegacyModule::handle_initialize(httpd_req_t* req) {
  JsonObject resp_json;
  control_task->running = false;
  resp_json.set("action", "initialize");
  resp_json.set("status", "success");
  send_resp(req, resp_json);
  return ESP_OK;
}

esp_err_t HttpLegacyModule::handle_assist_manual(httpd_req_t* req, JsonObject json) {
  auto left_torque_result = json.get_number("left_assistance_value");
  auto right_torque_result = json.get_number("right_assistance_value");
  auto active_leg_result = json.get_number("weak_leg");
  if (std::holds_alternative<JsonError>(left_torque_result)) {
    auto json_error = std::get<JsonError>(left_torque_result);
    LOGW("failed to read left_assistance_value %s", Json::get_json_error_string(json_error));
  }
  if (std::holds_alternative<JsonError>(right_torque_result)) {
    auto json_error = std::get<JsonError>(right_torque_result);
    LOGW("failed to read right_assistance_value %s", Json::get_json_error_string(json_error));
  }
  if (std::holds_alternative<JsonError>(active_leg_result)) {
    auto json_error = std::get<JsonError>(active_leg_result);
    LOGW("failed to read weak_leg %s", Json::get_json_error_string(json_error));
  }
  if (std::holds_alternative<JsonError>(left_torque_result) ||
      std::holds_alternative<JsonError>(right_torque_result) ||
      std::holds_alternative<JsonError>(active_leg_result)) {
    return ESP_OK;
  }
  auto left_torque = std::get<double>(left_torque_result);
  auto right_torque = std::get<double>(right_torque_result);
  auto active_leg = static_cast<LegacyLeg>(std::get<double>(active_leg_result));
  switch (active_leg) {
    case LegacyLeg::LEFT:
      right_torque = 0;
      break;
    case LegacyLeg::RIGHT:
      left_torque = 0;
      break;
    default:
      LOGW("invalid leg received");
      return ESP_OK;
  }
  control_task->control_mode = ControlMode::MANUAL;
  control_task->manual_controller.params.right.torque = right_torque;
  control_task->manual_controller.params.left.torque = left_torque;
  control_task->running = true;
  return ESP_OK;
}

esp_err_t HttpLegacyModule::handle_assist_automatic(httpd_req_t* req, JsonObject json) {
  auto left_torque_result = json.get_number("left_assistance_value");
  auto right_torque_result = json.get_number("right_assistance_value");
  auto right_assist_time_result = json.get_number("right_assistance_time");
  auto left_assist_time_result = json.get_number("left_assistance_time");
  if (std::holds_alternative<JsonError>(left_torque_result)) {
    auto json_error = std::get<JsonError>(left_torque_result);
    LOGW("failed to read left_assistance_value %s", Json::get_json_error_string(json_error));
  }
  if (std::holds_alternative<JsonError>(right_torque_result)) {
    auto json_error = std::get<JsonError>(right_torque_result);
    LOGW("failed to read right_assistance_value %s", Json::get_json_error_string(json_error));
  }
  if (std::holds_alternative<JsonError>(left_assist_time_result)) {
    auto json_error = std::get<JsonError>(left_assist_time_result);
    LOGW("failed to read left_assistance_time %s", Json::get_json_error_string(json_error));
  }
  if (std::holds_alternative<JsonError>(right_assist_time_result)) {
    auto json_error = std::get<JsonError>(right_assist_time_result);
    LOGW("failed to read right_assistance_time %s", Json::get_json_error_string(json_error));
  }

  if (std::holds_alternative<JsonError>(left_torque_result) ||
      std::holds_alternative<JsonError>(right_torque_result) ||
      std::holds_alternative<JsonError>(left_assist_time_result) ||
      std::holds_alternative<JsonError>(right_assist_time_result)) {
    return ESP_OK;
  }
  auto left_torque = std::get<double>(left_torque_result);
  auto right_torque = std::get<double>(right_torque_result);
  auto left_assist_time = std::get<double>(left_assist_time_result);
  auto right_assist_time = std::get<double>(right_assist_time_result);

  control_task->control_mode = ControlMode::AUTO;
  control_task->automatic_controller.params.right.torque = right_torque;
  control_task->automatic_controller.params.right.timeout = right_assist_time;
  control_task->automatic_controller.params.left.torque = left_torque;
  control_task->automatic_controller.params.left.timeout = left_assist_time;
  control_task->running = true;
  return ESP_OK;
}

esp_err_t HttpLegacyModule::handle_assist_semiautomatic(httpd_req_t* req, JsonObject json) {
  auto left_torque_result = json.get_number("left_assistance_value");
  auto right_torque_result = json.get_number("right_assistance_value");
  auto active_leg_result = json.get_number("weak_leg");
  auto right_assist_time_result = json.get_number("right_assistance_time");
  auto left_assist_time_result = json.get_number("left_assistance_time");
  auto left_delay_result = json.get_number("left_delay");
  auto right_delay_result = json.get_number("right_delay");
  if (std::holds_alternative<JsonError>(left_torque_result)) {
    auto json_error = std::get<JsonError>(left_torque_result);
    LOGW("failed to read left_assistance_value %s", Json::get_json_error_string(json_error));
  }
  if (std::holds_alternative<JsonError>(right_torque_result)) {
    auto json_error = std::get<JsonError>(right_torque_result);
    LOGW("failed to read right_assistance_value %s", Json::get_json_error_string(json_error));
  }
  if (std::holds_alternative<JsonError>(left_assist_time_result)) {
    auto json_error = std::get<JsonError>(left_assist_time_result);
    LOGW("failed to read left_assistance_time %s", Json::get_json_error_string(json_error));
  }
  if (std::holds_alternative<JsonError>(right_assist_time_result)) {
    auto json_error = std::get<JsonError>(right_assist_time_result);
    LOGW("failed to read right_assistance_time %s", Json::get_json_error_string(json_error));
  }
  if (std::holds_alternative<JsonError>(left_delay_result)) {
    auto json_error = std::get<JsonError>(left_delay_result);
    LOGW("failed to read left_delay %s", Json::get_json_error_string(json_error));
  }
  if (std::holds_alternative<JsonError>(right_delay_result)) {
    auto json_error = std::get<JsonError>(right_delay_result);
    LOGW("failed to read right_delay %s", Json::get_json_error_string(json_error));
  }
  if (std::holds_alternative<JsonError>(active_leg_result)) {
    auto json_error = std::get<JsonError>(active_leg_result);
    LOGW("failed to read weak_leg %s", Json::get_json_error_string(json_error));
  }
  if (std::holds_alternative<JsonError>(left_torque_result) ||
      std::holds_alternative<JsonError>(right_torque_result) ||
      std::holds_alternative<JsonError>(left_assist_time_result) ||
      std::holds_alternative<JsonError>(right_assist_time_result) ||
      std::holds_alternative<JsonError>(left_delay_result) ||
      std::holds_alternative<JsonError>(right_delay_result) ||
      std::holds_alternative<JsonError>(active_leg_result)) {
    return ESP_OK;
  }
  auto left_torque = std::get<double>(left_torque_result);
  auto right_torque = std::get<double>(right_torque_result);
  auto active_leg = static_cast<LegacyLeg>(std::get<double>(active_leg_result));
  auto left_assist_time = std::get<double>(left_assist_time_result);
  auto right_assist_time = std::get<double>(right_assist_time_result);
  auto left_delay = std::get<double>(left_delay_result);
  auto right_delay = std::get<double>(right_delay_result);
  switch (active_leg) {
    case LegacyLeg::LEFT:
      right_torque = 0;
      control_task->semiautomatic_controller.params.weak_leg = Leg::LEFT;
      break;
      case LegacyLeg::RIGHT:
      left_torque = 0;
      control_task->semiautomatic_controller.params.weak_leg = Leg::RIGHT;
      break;
    default:
      LOGW("invalid leg received");
      return ESP_OK;
  }
  control_task->control_mode = ControlMode::SEMI_AUTO;
  control_task->semiautomatic_controller.params.right.torque = right_torque;
  control_task->semiautomatic_controller.params.right.timeout = right_assist_time;
  control_task->semiautomatic_controller.params.right.delay = right_delay;
  control_task->semiautomatic_controller.params.left.torque = left_torque;
  control_task->semiautomatic_controller.params.left.timeout = left_assist_time;
  control_task->semiautomatic_controller.params.left.delay = left_delay;
  control_task->running = true;
  return ESP_OK;
}

esp_err_t HttpLegacyModule::handle_assist_smart(httpd_req_t* req, JsonObject json) {
  return ESP_OK;
}

esp_err_t HttpLegacyModule::handle_assist(httpd_req_t* req, JsonObject json) {
  auto mode_result = json.get_number("mode");
  if (std::holds_alternative<JsonError>(mode_result)) {
    auto json_error = std::get<JsonError>(mode_result);
    LOGW("failed to get assist mode %s", Json::get_json_error_string(json_error));
    return ESP_OK;
  }
  auto mode = static_cast<LegacyAssistMode>(std::get<double>(mode_result));
  switch (mode) {
    case LegacyAssistMode::MANUAL:
      return handle_assist_manual(req, json);
    case LegacyAssistMode::AUTOMATIC:
      return handle_assist_automatic(req, json);
    case LegacyAssistMode::SEMIAUTOMATIC:
      return handle_assist_semiautomatic(req, json);
    case LegacyAssistMode::SMART:
      return handle_assist_smart(req, json);
    default:
      LOGW("invalid mode received %d", mode);
      break;
  }
  return ESP_OK;
}

esp_err_t HttpLegacyModule::handle_release(httpd_req_t* req) {
  JsonObject resp_json;
  control_task->running = false;
  resp_json.set("action", "released");
  resp_json.set("status", "success");
  send_resp(req, resp_json);
  return ESP_OK;
};

esp_err_t HttpLegacyModule::handle_ping(httpd_req_t* req) {
  JsonObject resp_json;
  resp_json.set("action", "ping");
  resp_json.set("status", "ok");
  send_resp(req, resp_json);
  return ESP_OK;
}

esp_err_t HttpLegacyModule::handle_enable(httpd_req_t* req) {
  JsonObject resp_json;
  motor_task->enable();
  left_motor->zero_pos();
  right_motor->zero_pos();
  control_task->running = true;
  resp_json.set("action", "enable");
  resp_json.set("status", "success");
  send_resp(req, resp_json);
  return ESP_OK;
}

esp_err_t HttpLegacyModule::handle_disable(httpd_req_t* req) {
  JsonObject resp_json;
  control_task->running = false;
  motor_task->disable();
  resp_json.set("action", "disable");
  resp_json.set("status", "success");
  send_resp(req, resp_json);
  return ESP_OK;
}

void HttpLegacyModule::send_resp(httpd_req_t* req, JsonObject json) {
  auto json_str = json.stringify();
  httpd_ws_frame_t packet = {
      .final = true,
      .fragmented = false,
      .type = HTTPD_WS_TYPE_TEXT,
      .payload = reinterpret_cast<uint8_t*>(const_cast<char*>(json_str.c_str())),
      .len = json_str.length(),
  };
  httpd_ws_send_frame(req, &packet);
}

esp_err_t HttpLegacyModule::ws_post_handshake(httpd_req_t* req) {
  LOGI("connected");
  return ESP_OK;
}

void HttpLegacyModule::register_direct_uris() {
  register_ws_uri("/", ws, ws_post_handshake);
}