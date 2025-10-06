#pragma once

#include "config.hpp"
#include "ipc/queue.hpp"
#include "led/modes.hpp"
#include "service.hpp"

class LedService : public Service<config::service::led::stack_size> {
 private:
  static void main(LedService* self);

 public:
  Queue<LedStatus, config::service::led::queue_len> queue;
  void start();
};