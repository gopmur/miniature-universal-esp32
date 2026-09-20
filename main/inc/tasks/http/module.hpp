#pragma once

#include <cstddef>
#include <string>
#include <vector>
#include "esp_http_server.h"
#include "loggable.hpp"

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

  protected:
  void register_uri(const char* uri_address,
                    httpd_method_t method,
                    esp_err_t (*handler)(httpd_req_t* req));
  void register_ws_uri(const char* uri_address,
                       esp_err_t (*handler)(httpd_req_t* req),
                       esp_err_t (*post_handshake_handler)(httpd_req_t* req));

  void register_uri_with_option(const char* uri_address,
                                httpd_method_t method,
                                esp_err_t (*handler)(httpd_req_t* req));

  static void allow_cors(httpd_req_t* req);
  static void set_close_connection(httpd_req_t* req);
  static void set_type_json(httpd_req_t* req);
  static void set_header(httpd_req_t* req);
  static bool check_uri(const char* uri);

  public:
  void register_uris(httpd_handle_t server_instance);
  HttpModule(const char* name, std::vector<HttpModule*> modules);
  HttpModule(const char* name);
};