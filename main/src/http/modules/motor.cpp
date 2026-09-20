#include "http/modules/motor.hpp"
#include "custom_drivers/motor.hpp"

extern AbstractMotorDriver* left_motor;
extern AbstractMotorDriver* right_motor;

esp_err_t HttpMotorModule::get_zero_pos(httpd_req_t* req) {
  set_header(req);
  left_motor->zero_pos();
  right_motor->zero_pos();
  httpd_resp_send(req, nullptr, 0);
  return ESP_OK;
}

void HttpMotorModule::register_direct_uris() {
  register_uri("/zero-pos", HTTP_GET, get_zero_pos);
}