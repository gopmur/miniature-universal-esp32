#pragma once

#include "config.hpp"
#include "ipc/queue.hpp"
#include "service.hpp"
#include "services/stm_uart/lappl.hpp"

class WebSocketService : public Service<config::service::ws::stack_size> {
  private:
  static void main(WebSocketService* self);
  std::array<int, config::service::ws::max_connection> connection_fds;
  std::array<int, config::service::ws::max_connection> connection_age;
  int connection_count;

  public:
  Queue<LapplPacket, 32> queue;
  WebSocketService();
  bool has_connections();
  void start();
  void start_sending(int fd);
  void stop_sending(int fd);
};