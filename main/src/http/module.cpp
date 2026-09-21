#include "http/module.hpp"
#include <string>
#include "esp_err.h"
#include "esp_http_server.h"

HttpModule::HttpModule(const char* name) : name(name) {};

HttpModule::HttpModule(const char* name, std::vector<HttpModule*> modules)
    : name(name), modules(modules) {};

void HttpModule::register_modules_uris() {
  for (auto module : modules) {
    module->base_uri = base_uri + name + "/";
    module->register_uris(this->server_instance);
  }
}

void HttpModule::register_uris(httpd_handle_t server_instance) {
  this->server_instance = server_instance;
  register_modules_uris();
  register_direct_uris();
}

void HttpModule::register_uri(const char* uri_address,
                              httpd_method_t method,
                              esp_err_t (*handler)(httpd_req_t* req)) {
  if (!check_uri(uri_address)) {
    return;
  }
  auto full_uri_address = new std::string(base_uri + name + uri_address);
  httpd_uri uri = {
      .uri = full_uri_address->c_str(),
      .method = method,
      .handler = handler,
      .user_ctx = nullptr,
      .is_websocket = false,
      .handle_ws_control_frames = false,
      .supported_subprotocol = nullptr,
      .ws_post_handshake_cb = nullptr,
  };
  ESP_ERROR_CHECK(httpd_register_uri_handler(this->server_instance, &uri));
  LOGI("uri address registered %s", full_uri_address->c_str());
}

void HttpModule::register_ws_uri(const char* uri_address,
                                 esp_err_t (*handler)(httpd_req_t* req),
                                 esp_err_t (*post_handshake_handler)(httpd_req_t* req)) {
  if (!check_uri(uri_address)) {
    return;
  }
  auto full_uri_address = new std::string(base_uri + name + uri_address);
  httpd_uri_t uri = {
      .uri = full_uri_address->c_str(),
      .method = HTTP_GET,
      .handler = handler,
      .user_ctx = nullptr,
      .is_websocket = true,
      .handle_ws_control_frames = false,
      .supported_subprotocol = nullptr,
      .ws_post_handshake_cb = post_handshake_handler,

  };
  ESP_ERROR_CHECK(httpd_register_uri_handler(this->server_instance, &uri));
  LOGI("uri address registered %s", full_uri_address->c_str());
}

void HttpModule::register_uri_with_option(const char* uri_address,
                                          httpd_method_t method,
                                          esp_err_t (*handler)(httpd_req_t* req)) {
  if (!check_uri(uri_address)) {
    return;
  }
  auto full_uri_address = new std::string(base_uri + name + uri_address);
  httpd_uri uri = {
      .uri = full_uri_address->c_str(),
      .method = method,
      .handler = handler,
      .user_ctx = nullptr,
      .is_websocket = false,
      .handle_ws_control_frames = false,
      .supported_subprotocol = nullptr,
      .ws_post_handshake_cb = nullptr,
  };

  httpd_uri option_uri = {
      .uri = uri_address,
      .method = HTTP_OPTIONS,
      .handler = options_handler,
      .user_ctx = nullptr,
      .is_websocket = false,
      .handle_ws_control_frames = false,
      .supported_subprotocol = nullptr,
      .ws_post_handshake_cb = nullptr,
  };

  ESP_ERROR_CHECK(httpd_register_uri_handler(this->server_instance, &uri));
  ESP_ERROR_CHECK(httpd_register_uri_handler(this->server_instance, &option_uri));
  LOGI("uri address registered %s", full_uri_address->c_str());
}

esp_err_t HttpModule::options_handler(httpd_req_t* req) {
  set_header(req);
  httpd_resp_send(req, NULL, 0);
  return ESP_OK;
}

void HttpModule::allow_cors(httpd_req_t* req) {
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "GET, PUT, POST, OPTIONS");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "Content-Type");
}

void HttpModule::set_close_connection(httpd_req_t* req) {
  httpd_resp_set_hdr(req, "Connection", "close");
}

void HttpModule::set_type_json(httpd_req_t* req) {
  httpd_resp_set_type(req, "application/json");
}

void HttpModule::set_header(httpd_req_t* req) {
  allow_cors(req);
  set_close_connection(req);
  set_type_json(req);
}

bool HttpModule::check_uri(const char* uri) {
  if (strlen(uri) == 0) {
    LOGE("uri cannot be empty");
    return false;
  }
  if (uri[0] != '/') {
    LOGW("'%s' uri does not start with /. this may not be wat you intended to do", uri);
  }
  return true;
}

esp_err_t HttpModule::send_json(httpd_req_t* req, JsonObject& json) {
  auto json_string = json.stringify();
  httpd_resp_send(req, json_string.c_str(), HTTPD_RESP_USE_STRLEN);
  return ESP_OK;
}

esp_err_t HttpModule::send_json(httpd_req_t* req, JsonObject& json, httpd_err_code_t status) {
  auto json_string = json.stringify();
  return httpd_resp_send_err(req, status, json_string.c_str());
}