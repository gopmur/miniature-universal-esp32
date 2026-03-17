#pragma once

#include "config.hpp"
#include "service.hpp"

class MonitorService
    : public AbstractService<MonitorService,
                             config::service::monitor::stack_size> {
  public:
  void main();
  MonitorService(int priority);

};