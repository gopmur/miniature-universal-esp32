#pragma once

#include "esp_http_server.h"
#include "services/http.hpp"
#include "thread.hpp"

class GetStates : public ThreadWithArg<GetStates, httpd_req_t*> {
  private:
  const char* get_contorl_mode_str(ControlMode control_mode);
  public:
  GetStates(const char* name, int priority, int stack_size);
  void main(httpd_req_t** req_p);
};