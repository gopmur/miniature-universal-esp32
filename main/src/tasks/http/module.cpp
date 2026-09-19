#include "tasks/http/module.hpp"
#include <string>
#include "esp_err.h"
#include "esp_http_server.h"
#include "esp_log.h"

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
  ESP_LOGI("http module", "uri address registered %s", full_uri_address->c_str());
}