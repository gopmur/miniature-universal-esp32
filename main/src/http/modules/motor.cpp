#include "http/modules/motor.hpp"
#include "tasks/motor.hpp"

extern MotorTask* motor_task;

esp_err_t HttpMotorModule::get_zero_pos(httpd_req_t* req) {
  motor_task->zero_pos_all();
  return send_success_json(req);
}

void HttpMotorModule::register_direct_uris() {
  register_uri("/zero-pos", HTTP_GET, get_zero_pos);
}