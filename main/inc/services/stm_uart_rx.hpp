#pragma once

#include "config.hpp"
#include "hal/uart_types.h"
#include "service.hpp"
#include "services/http.hpp"
#include "services/stm_uart/ssp.hpp"

class StmUartRxService : public Service<config::service::stm_uart::stack_size> {
  private:
  uint8_t rx_buffer[config::stm_uart::rx_buffer_size];
  const uart_port_t port;
  std::optional<HttpQueueMessageHeader> ssp_address_to_http_queue_message_header(SspAddress ssp_address);

  public:
  void main();
  StmUartRxService(int priority, uart_port_t port);
};