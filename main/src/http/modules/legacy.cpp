#include "http/modules/legacy.hpp"
#include <cstdio>
#include <variant>
#include "esp_http_server.h"
#include "jayson.hpp"
#include "system_logger.hpp"

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
  }
  return ESP_OK;
}

esp_err_t HttpLegacyModule::handle_ping(httpd_req_t* req) {
  JsonObject resp_json;
  resp_json.set("ping", "ok");
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