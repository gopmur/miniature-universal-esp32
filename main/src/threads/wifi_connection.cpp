#include "threads/wifi_connection.hpp"
#include <variant>
#include "esp_err.h"
#include "esp_http_server.h"
#include "esp_wifi.h"
#include "esp_wifi_types_generic.h"
#include "helper/json.hpp"

WifiConnectionThread::WifiConnectionThread(const char* name,
                                           int priority,
                                           int stack_size)
    : ThreadWithArg(name, priority, stack_size) {}

void WifiConnectionThread::main(httpd_req_t** req_p) {
  auto req = *req_p;

  char* req_body = static_cast<char*>(malloc(req->content_len + 1));

  int total = 0;
  int remaining = req->content_len;

  while (remaining > 0) {
    int r = httpd_req_recv(req, req_body + total, remaining);
    if (r <= 0) {
      free(req_body);
      httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "recv failed");
      httpd_req_async_handler_complete(req);
      return;
    }
    total += r;
    remaining -= r;
  }

  req_body[total] = 0;
  JsonObject error_json;

  auto req_json_result = JsonObject::parse(req_body);
  if (std::holds_alternative<JsonError>(req_json_result)) {
    error_json.set_string("message", "parse error");
    auto res_str = error_json.stringify();
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, res_str);
    free(res_str);
    free(req_body);
    return;
  }

  auto req_json = std::get<JsonObject>(req_json_result);

  free(req_body);
  auto ssid_result = req_json.get_string("ssid", &error_json);
  auto password_result = req_json.get_string("password", &error_json);
  wifi_config_t sta_config = {};

  if (std::holds_alternative<char*>(ssid_result)) {
    auto ssid = std::get<char*>(ssid_result);
    int ssid_len = strlen(ssid);
    if (ssid_len >= 32) {
      error_json.set_string("ssid", "too long");
    } else {
      std::strcpy(reinterpret_cast<char*>(sta_config.sta.ssid), ssid);
    }
  }

  if (std::holds_alternative<char*>(password_result)) {
    auto password = std::get<char*>(password_result);
    int password_len = strlen(password);
    if (password_len >= 32) {
      error_json.set_string("password", "too long");
    } else {
      std::strcpy(reinterpret_cast<char*>(sta_config.sta.password), password);
    }
  }

  if (!error_json.is_empty()) {
    auto res_str = error_json.stringify();
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, res_str);
    free(res_str);
  }

  else {
    sta_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    ESP_ERROR_CHECK(esp_wifi_disconnect());
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_config));
    ESP_ERROR_CHECK(esp_wifi_connect());
    httpd_resp_send(req, nullptr, 0);
  }

  httpd_req_async_handler_complete(req);
}