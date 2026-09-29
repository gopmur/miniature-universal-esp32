#pragma once

#include <cstddef>
#include <string>
#include <vector>
#include "esp_http_server.h"
#include "jayson.hpp"
#include "system_logger.hpp"

class HttpModule {
  MAKE_LOGGABLE("http_module");

  private:
  std::string name;
  std::string base_uri = "/";
  httpd_handle_t server_instance;
  std::vector<HttpModule*> modules;
  void register_modules_uris();
  virtual void register_direct_uris() = 0;
  static esp_err_t options_handler(httpd_req_t* req);
  static void set_global_header(httpd_req_t* req);
  static esp_err_t middleware(httpd_req_t* req);
  static esp_err_t json_middleware(httpd_req_t* req);
  static bool check_uri(const char* uri);
  static bool check_content_len(httpd_req_t* req);

  protected:
  void register_uri(const char* uri_address,
                    httpd_method_t method,
                    esp_err_t (*handler)(httpd_req_t* req));
  void register_uri(const char* uri_address,
                    httpd_method_t method,
                    esp_err_t (*handler)(httpd_req_t* req, JsonObject* req_json));
  void register_ws_uri(const char* uri_address,
                       esp_err_t (*handler)(httpd_req_t* req),
                       esp_err_t (*post_handshake_handler)(httpd_req_t* req));

  void register_uri_with_option(const char* uri_address,
                                httpd_method_t method,
                                esp_err_t (*handler)(httpd_req_t* req));
  void register_uri_with_option(const char* uri_address,
                                httpd_method_t method,
                                esp_err_t (*handler)(httpd_req_t* req, JsonObject* req_json));

  static void allow_cors(httpd_req_t* req);
  static void set_keep_alive(httpd_req_t* req);
  static void set_type_json(httpd_req_t* req);
  static esp_err_t send_json(httpd_req_t* req, JsonObject& json);
  static esp_err_t send_json(httpd_req_t* req, JsonObject& json, httpd_err_code_t status);
  static esp_err_t send_message_json(httpd_req_t* req, const char* message);
  static esp_err_t send_message_json(httpd_req_t* req,
                                     const char* message,
                                     httpd_err_code_t status);
  static esp_err_t send_success_json(httpd_req_t* req);
  static std::variant<JsonObject, JsonError> parse_json(httpd_req_t* req);

  public:
  void register_uris(httpd_handle_t server_instance);
  HttpModule(const char* name, std::vector<HttpModule*> modules);
  HttpModule(const char* name);
};