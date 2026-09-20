#pragma once

#include "tasks/http/module.hpp"

class HttpStreamModule : public HttpModule {
  private:
  static esp_err_t ws_imu(httpd_req_t* req);
  static esp_err_t ws_imu_post_handshake(httpd_req_t* req);
  static esp_err_t ws_motor(httpd_req_t* req);
  static esp_err_t ws_motor_post_handshake(httpd_req_t* req);
  static esp_err_t ws_tasks(httpd_req_t* req);
  static esp_err_t ws_tasks_post_handshake(httpd_req_t* req);
  static esp_err_t ws_ota(httpd_req_t* req);
  static esp_err_t ws_ota_post_handshake(httpd_req_t* req);
  void register_direct_uris();

  public:
  HttpStreamModule(const char* name, std::vector<HttpModule*> modules);
  HttpStreamModule(const char* name);
};