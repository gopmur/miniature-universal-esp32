#pragma once

#include "config.hpp"
#include "ipc/queue.hpp"
#include "service.hpp"

class WebSocketService : public Service<config::service::ws::stack_size> {
  private:
  static void main(WebSocketService* self);
  int fd;

  public:
  Queue<float, 8> queue;
  WebSocketService();
  void start();
  void start_sending(int fd);
  void stop_sending();
};