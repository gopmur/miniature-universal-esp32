#pragma once

#include "config.hpp"
#include "hal/uart_types.h"
#include "service.hpp"
#include "stm_uart/packet.hpp"

class StmUartRxService : public Service<config::service::stm_uart::stack_size> {
  private:
  uint8_t rx_buffer;
  const uart_port_t port;

  static void main(StmUartRxService* service);

  public:
  StmUartRxService(uart_port_t port);

  void start();
};