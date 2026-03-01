#pragma once

#include "config.hpp"
#include "ipc/queue.hpp"
#include "service.hpp"
#include "services/stm_uart/lappl.hpp"

class WebSocketService : public Service<config::service::ws::stack_size> {
  private:
  static void main(WebSocketService* self);
  int fd;

  public:
  Queue<LapplPacket, 8> queue;
  WebSocketService();
  bool is_connected();
  void start();
  void start_sending(int fd);
  void stop_sending();
};