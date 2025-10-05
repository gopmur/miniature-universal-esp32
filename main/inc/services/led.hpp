#pragma once

#include "config.hpp"
#include "service.hpp"

class LedService : public Service<config::service::led::stack_size> {
 private:
  static void main(LedService* self);

 public:
  void start();
};