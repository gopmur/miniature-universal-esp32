#pragma once

#include "config.hpp"
#include "service.hpp"

class MonitorService : public AbstractService<config::service::monitor::stack_size> {
  private:
  static void main(MonitorService* self);

  public:
  MonitorService(int priority);
  void start();
};