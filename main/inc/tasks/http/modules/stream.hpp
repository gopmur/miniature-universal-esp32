// #pragma once

// #include "helper/json.hpp"
// #include "tasks/control.hpp"
// #include "tasks/http/helper.hpp"
// #include "tasks/http/module.hpp"

// extern ControlTask* control_task;

// class HttpControlModule : public HttpModule {
//   private:
//   static const char* get_control_mode_str(ControlMode control_mode);
//   static const char* get_leg_str(Leg leg);
//   static esp_err_t get_params(httpd_req_t* req);
//   static esp_err_t put_start(httpd_req_t* req);
//   static esp_err_t put_stop(httpd_req_t* req);
//   static esp_err_t put_set_mode_manual(httpd_req_t* req);
//   static esp_err_t put_set_mode_automatic(httpd_req_t* req);
//   static esp_err_t put_set_mode_semiautomatic(httpd_req_t* req);
//   static esp_err_t put_set_mode_smart(httpd_req_t* req);

//   public:
//   HttpControlModule(const char* name, std::vector<HttpModule*> modules);
//   HttpControlModule(const char* name);
// };