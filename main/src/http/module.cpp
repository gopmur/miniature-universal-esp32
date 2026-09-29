#include "http/module.hpp"
#include <format>
#include <string>
#include <variant>
#include "esp_err.h"
#include "esp_http_server.h"
#include "http_parser.h"
#include "jayson.hpp"
#include "system_logger.hpp"

HttpModule::HttpModule(const char* name) : name(name) {};

HttpModule::HttpModule(const char* name, std::vector<HttpModule*> modules)
    : name(name), modules(modules) {};

void HttpModule::set_global_header(httpd_req_t* req) {
#ifdef CONFIG_HEXA_HTTP_ALLOW_CORS
  allow_cors(req);
#endif
#ifdef CONFIG_HEXA_HTTP_KEEP_ALIVE
  set_keep_alive(req);
#endif
}

esp_err_t HttpModule::middleware(httpd_req_t* req) {
  set_global_header(req);
  if (!check_content_len(req)) {
    JsonObject resp;
    auto message =
        std::format("content length is longer than {}", CONFIG_HEXA_HTTP_MAX_REQUEST_LEN);
    resp.set("message", message.c_str());
    return send_json(req, resp, HTTPD_413_CONTENT_TOO_LARGE);
  }
  auto handler = reinterpret_cast<esp_err_t (*)(httpd_req_t*)>(req->user_ctx);
  if (handler == nullptr) {
    LOGE("empty handler while trying to call from middleware");
    return ESP_OK;
  }
  return handler(req);
}

esp_err_t HttpModule::json_middleware(httpd_req_t* req) {
  set_global_header(req);
  if (!check_content_len(req)) {
    JsonObject resp;
    auto message =
        std::format("content length is longer than {}", CONFIG_HEXA_HTTP_MAX_REQUEST_LEN);
    resp.set("message", message.c_str());
    return send_json(req, resp, HTTPD_413_CONTENT_TOO_LARGE);
  }
  auto handler = reinterpret_cast<esp_err_t (*)(httpd_req_t*, JsonObject*)>(req->user_ctx);

  auto req_json_result = parse_json(req);
  if (std::holds_alternative<JsonError>(req_json_result)) {
    send_message_json(req, "json parse error", HTTPD_400_BAD_REQUEST);
    return ESP_OK;
  }
  auto req_json = std::get<JsonObject>(req_json_result);
  return handler(req, &req_json);
}

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
  if (strlen(uri_address) == 1) {
    uri_address = "";
  }
  auto full_uri_address = new std::string(base_uri + name + uri_address);
  httpd_uri uri = {
      .uri = full_uri_address->c_str(),
      .method = method,
      .handler = middleware,
      .user_ctx = reinterpret_cast<void*>(handler),
      .is_websocket = false,
      .handle_ws_control_frames = false,
      .supported_subprotocol = nullptr,
      .ws_post_handshake_cb = nullptr,
  };
  ESP_ERROR_CHECK(httpd_register_uri_handler(this->server_instance, &uri));
  LOGI("uri address registered %s", full_uri_address->c_str());
}

void HttpModule::register_uri(const char* uri_address,
                              httpd_method_t method,
                              esp_err_t (*handler)(httpd_req_t* req, JsonObject* req_json)) {
  if (method == HTTP_GET) {
    LOGW("GET handlers cannot be registered via json_middleware. %s registration ignored",
         uri_address);
    return;
  }
  if (!check_uri(uri_address)) {
    return;
  }
  if (strlen(uri_address) == 1) {
    uri_address = "";
  }
  auto full_uri_address = new std::string(base_uri + name + uri_address);
  httpd_uri uri = {
      .uri = full_uri_address->c_str(),
      .method = method,
      .handler = middleware,
      .user_ctx = reinterpret_cast<void*>(handler),
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
  if (strlen(uri_address) == 1) {
    uri_address = "";
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
  if (strlen(uri_address) == 1) {
    uri_address = "";
  }
  auto full_uri_address = new std::string(base_uri + name + uri_address);
  httpd_uri uri = {
      .uri = full_uri_address->c_str(),
      .method = method,
      .handler = middleware,
      .user_ctx = reinterpret_cast<void*>(handler),
      .is_websocket = false,
      .handle_ws_control_frames = false,
      .supported_subprotocol = nullptr,
      .ws_post_handshake_cb = nullptr,
  };

  httpd_uri option_uri = {
      .uri = uri_address,
      .method = HTTP_OPTIONS,
      .handler = middleware,
      .user_ctx = reinterpret_cast<void*>(options_handler),
      .is_websocket = false,
      .handle_ws_control_frames = false,
      .supported_subprotocol = nullptr,
      .ws_post_handshake_cb = nullptr,
  };

  ESP_ERROR_CHECK(httpd_register_uri_handler(this->server_instance, &uri));
  ESP_ERROR_CHECK(httpd_register_uri_handler(this->server_instance, &option_uri));
  LOGI("uri address registered %s", full_uri_address->c_str());
}

void HttpModule::register_uri_with_option(const char* uri_address,
                                          httpd_method_t method,
                                          esp_err_t (*handler)(httpd_req_t* req,
                                                               JsonObject* req_json)) {
  if (!check_uri(uri_address)) {
    return;
  }
  if (strlen(uri_address) == 1) {
    uri_address = "";
  }
  auto full_uri_address = new std::string(base_uri + name + uri_address);
  httpd_uri uri = {
      .uri = full_uri_address->c_str(),
      .method = method,
      .handler = json_middleware,
      .user_ctx = reinterpret_cast<void*>(handler),
      .is_websocket = false,
      .handle_ws_control_frames = false,
      .supported_subprotocol = nullptr,
      .ws_post_handshake_cb = nullptr,
  };

  httpd_uri option_uri = {
      .uri = uri_address,
      .method = HTTP_OPTIONS,
      .handler = middleware,
      .user_ctx = reinterpret_cast<void*>(options_handler),
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
  httpd_resp_send(req, NULL, 0);
  return ESP_OK;
}

void HttpModule::allow_cors(httpd_req_t* req) {
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "GET, PUT, POST, OPTIONS");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "Content-Type");
}

void HttpModule::set_keep_alive(httpd_req_t* req) {
  httpd_resp_set_hdr(req, "Connection", "keep-alive");
}

void HttpModule::set_type_json(httpd_req_t* req) {
  httpd_resp_set_type(req, "application/json");
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

bool HttpModule::check_content_len(httpd_req_t* req) {
  return req->content_len <= CONFIG_HEXA_HTTP_MAX_REQUEST_LEN;
}

esp_err_t HttpModule::send_json(httpd_req_t* req, Json& json) {
  set_type_json(req);
  auto json_string = json.stringify();
  httpd_resp_send(req, json_string.c_str(), HTTPD_RESP_USE_STRLEN);
  return ESP_OK;
}

esp_err_t HttpModule::send_json(httpd_req_t* req, Json& json, httpd_err_code_t status) {
  set_type_json(req);
  auto json_string = json.stringify();
  return httpd_resp_send_err(req, status, json_string.c_str());
}

std::variant<JsonObject, JsonError> HttpModule::parse_json(httpd_req_t* req) {
  char* req_body = new char[req->content_len];
  httpd_req_recv(req, req_body, req->content_len);
  auto req_json_result = JsonObject::parse(req_body);
  delete[] req_body;
  return req_json_result;
}

esp_err_t HttpModule::send_message_json(httpd_req_t* req, const char* message) {
  JsonObject resp;
  resp.set("message", message);
  return send_json(req, resp);
}

esp_err_t HttpModule::send_message_json(httpd_req_t* req,
                                        const char* message,
                                        httpd_err_code_t status) {
  JsonObject resp;
  resp.set("message", message);
  return send_json(req, resp, status);
}

esp_err_t HttpModule::send_success_json(httpd_req_t* req) {
  return send_message_json(req, "success");
}