#pragma once

#include "config.hpp"
#include "hal/uart_types.h"
#include "service.hpp"

class StmUartRxService : public AbstractService<StmUartRxService, config::service::stm_uart::stack_size> {
  private:
  uint8_t rx_buffer[1024];
  const uart_port_t port;

  
  public:
  void main();
  StmUartRxService(int priority, uart_port_t port);
};