#include "threads/get_states.hpp"
#include "context/services/http.hpp"
#include "esp_http_server.h"
#include "esp_log.h"
#include "helper/json.hpp"
#include "services/http.hpp"

const char* GetStates::get_control_mode_str(ControlMode control_mode) {
  switch (control_mode) {
    case ControlMode::MANUAL:
      return "manual";
    case ControlMode::AUTO:
      return "automatic";
    case ControlMode::SEMI_AUTO:
      return "semi-automatic";
    case ControlMode::SMART:
      return "smart";
  }
}

std::optional<const char*> GetStates::get_json_key(
    HttpQueueMessageHeader http_queue_message_header) {
  switch (http_queue_message_header) {
    case HttpQueueMessageHeader::RUNNING:
      return "running";
    case HttpQueueMessageHeader::MODE:
      return "mode";
    case HttpQueueMessageHeader::SEMIAUTOMATIC_WEAK_LEG:
      return "weakLeg";
    case HttpQueueMessageHeader::RIGHT_MANUAL_TORQUE:
    case HttpQueueMessageHeader::LEFT_MANUAL_TORQUE:
    case HttpQueueMessageHeader::AUTOMATIC_LEFT_TORQUE:
    case HttpQueueMessageHeader::AUTOMATIC_RIGHT_TORQUE:
    case HttpQueueMessageHeader::SEMIAUTOMATIC_LEFT_TORQUE:
    case HttpQueueMessageHeader::SEMIAUTOMATIC_RIGHT_TORQUE:
    case HttpQueueMessageHeader::SMART_LEFT_TORQUE:
    case HttpQueueMessageHeader::SMART_RIGHT_TORQUE:
      return "torque";
    case HttpQueueMessageHeader::AUTOMATIC_LEFT_TIMEOUT:
    case HttpQueueMessageHeader::AUTOMATIC_RIGHT_TIMEOUT:
    case HttpQueueMessageHeader::SEMIAUTOMATIC_LEFT_TIMEOUT:
    case HttpQueueMessageHeader::SEMIAUTOMATIC_RIGHT_TIMEOUT:
      return "timeout";
    case HttpQueueMessageHeader::AUTOMATIC_LEFT_VELOCITY_THRESHOLD:
    case HttpQueueMessageHeader::AUTOMATIC_RIGHT_VELOCITY_THRESHOLD:
      return "velocityThreshold";
    case HttpQueueMessageHeader::SEMIAUTOMATIC_START_ASSIST_ANGLE:
      return "startAssistAngle";
    case HttpQueueMessageHeader::SEMIAUTOMATIC_STOP_ASSIST_ANGLE:
      return "stopAssistAngle";
    case HttpQueueMessageHeader::SEMIAUTOMATIC_LEFT_DELAY:
    case HttpQueueMessageHeader::SEMIAUTOMATIC_RIGHT_DELAY:
      return "delay";
    default:
      return std::nullopt;
  }
}

GetStates::GetStates(const char* name, int priority, int stack_size)
    : ThreadWithArg(name, priority, stack_size) {}

void GetStates::main(httpd_req_t** req_p) {
  auto req = *req_p;
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

  for (int i = 0; i < 21; i++) {
    auto response = http_service.state_queue.receive(1000);
    if (!response.has_value()) {
      res_json.set("message", "timed out");
      auto res_str = res_json.stringify();
      httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, res_str.c_str());
      httpd_req_async_handler_complete(req);
      return;
    }
    auto json_key_result = get_json_key(response->header);
    if (!json_key_result.has_value()) {
      break;  // ! we gotta like do something here idk
    }
    auto json_key = json_key_result.value();
    switch (response->header) {
      case HttpQueueMessageHeader::RUNNING:
        res_json.set("running", response->payload.b);
        break;
      case HttpQueueMessageHeader::MODE:
        res_json.set("mode", get_control_mode_str(response->payload.control_mode));
        break;
      case HttpQueueMessageHeader::LEFT_MANUAL_TORQUE:
        manual_control_params_left_json.set(json_key, response->payload.f);
        break;
      case HttpQueueMessageHeader::RIGHT_MANUAL_TORQUE:
        manual_control_params_right_json.set(json_key, response->payload.f);
        break;
      case HttpQueueMessageHeader::AUTOMATIC_LEFT_TORQUE:
      case HttpQueueMessageHeader::AUTOMATIC_LEFT_TIMEOUT:
      case HttpQueueMessageHeader::AUTOMATIC_LEFT_VELOCITY_THRESHOLD:
        automatic_control_params_left_json.set(json_key, response->payload.f);
        break;
      case HttpQueueMessageHeader::AUTOMATIC_RIGHT_TORQUE:
      case HttpQueueMessageHeader::AUTOMATIC_RIGHT_TIMEOUT:
      case HttpQueueMessageHeader::AUTOMATIC_RIGHT_VELOCITY_THRESHOLD:
        automatic_control_params_right_json.set(json_key, response->payload.f);
        break;
      case HttpQueueMessageHeader::SEMIAUTOMATIC_WEAK_LEG: {
        auto leg_str = response->payload.leg == Leg::LEFT ? "left" : "right";
        semiautomatic_control_params_json.set(json_key, leg_str);
        break;
      }
      case HttpQueueMessageHeader::SEMIAUTOMATIC_LEFT_TORQUE:
      case HttpQueueMessageHeader::SEMIAUTOMATIC_LEFT_TIMEOUT:
      case HttpQueueMessageHeader::SEMIAUTOMATIC_LEFT_DELAY:
        semiautomatic_control_params_left_json.set(json_key, response->payload.f);
        break;
      case HttpQueueMessageHeader::SEMIAUTOMATIC_RIGHT_TORQUE:
      case HttpQueueMessageHeader::SEMIAUTOMATIC_RIGHT_TIMEOUT:
      case HttpQueueMessageHeader::SEMIAUTOMATIC_RIGHT_DELAY:
        semiautomatic_control_params_right_json.set(json_key, response->payload.f);
        break;
      case HttpQueueMessageHeader::SEMIAUTOMATIC_START_ASSIST_ANGLE:
      case HttpQueueMessageHeader::SEMIAUTOMATIC_STOP_ASSIST_ANGLE:
        semiautomatic_control_params_json.set(json_key, response->payload.f);
        break;
      case HttpQueueMessageHeader::SMART_LEFT_TORQUE:
        smart_control_params_left_json.set(json_key, response->payload.f);
        break;
      case HttpQueueMessageHeader::SMART_RIGHT_TORQUE:
        smart_control_params_right_json.set(json_key, response->payload.f);
        break;
      default:
        break;
    }
  }

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
  auto ret = httpd_resp_send(req, json_str.c_str(), HTTPD_RESP_USE_STRLEN);
  if (ret != ESP_OK) {
    ESP_LOGE(HttpService::LOG_TAG, "Get running response transmission failed");
  }
  httpd_req_async_handler_complete(req);
}