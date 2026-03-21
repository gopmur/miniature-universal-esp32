#include "threads/get_states.hpp"
#include "context/services/http.hpp"
#include "esp_http_server.h"
#include "esp_log.h"
#include "helper/json.hpp"
#include "portmacro.h"
#include "services/http.hpp"

const char* GetStates::get_contorl_mode_str(ControlMode control_mode) {
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

GetStates::GetStates(const char* name, int priority, int stack_size)
    : ThreadWithArg(name, priority, stack_size) {}

void GetStates::main(httpd_req_t** req_p) {
  auto req = *req_p;
  JsonObject res_json;

  for (int i = 0; i < 4; i++) {
    auto response = http_service.state_queue.receive(portMAX_DELAY);
    if (!response.has_value()) {
      httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "STM32 not responding");
      httpd_req_async_handler_complete(req);
      return;
    }
    switch (response->header) {
      case HttpQueueMessageHeader::RUNNING:
        res_json.set_bool("running", response->payload.b);
        break;
      case HttpQueueMessageHeader::LEFT_MANUAL_TORQUE:
        res_json.set_number("leftTorque", response->payload.f);
        break;
      case HttpQueueMessageHeader::RIGHT_MANUAL_TORQUE:
        res_json.set_number("rightTorque", response->payload.f);
        break;
      case HttpQueueMessageHeader::MODE:
        res_json.set_string("mode", get_contorl_mode_str(response->payload.control_mode));
        break;
      default:
        break;
    }
  }

  auto json_str = res_json.stringify();
  auto ret = httpd_resp_send(req, json_str, HTTPD_RESP_USE_STRLEN);
  if (ret != ESP_OK) {
    ESP_LOGE(HttpService::LOG_TAG, "Get running response transmission failed");
  }
  free(json_str);
  httpd_req_async_handler_complete(req);
}