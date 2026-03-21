#pragma once

#include "config.hpp"
#include "ipc/queue.hpp"
#include "led/modes.hpp"
#include "service.hpp"

class LedService
    : public Service<config::service::led::stack_size> {
  public:
  void main();
  LedService(int priority);
  Queue<LedStatus, config::service::led::queue_len> queue;
};