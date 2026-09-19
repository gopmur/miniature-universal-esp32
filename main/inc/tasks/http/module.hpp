#pragma once

#include <string>
#include <vector>
#include "esp_http_server.h"

class HttpModule {
  private:
  std::string name;
  std::string base_uri = "/";
  httpd_handle_t server_instance;
  std::vector<HttpModule*> modules;
  void register_modules_uris();
  virtual void register_direct_uris() = 0;

  protected:
  void register_uri(const char* uri_address,
                    httpd_method_t method,
                    esp_err_t (*handler)(httpd_req_t* req));

  public:
  void register_uris(httpd_handle_t server_instance);
  HttpModule(const char* name, std::vector<HttpModule*> modules);
  HttpModule(const char* name);
};