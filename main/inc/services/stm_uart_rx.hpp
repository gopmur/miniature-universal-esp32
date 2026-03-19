#pragma once

#include "config.hpp"
#include "hal/uart_types.h"
#include "service.hpp"
#include "services/http.hpp"
#include "services/stm_uart/rssp.hpp"

class StmUartRxService
    : public AbstractService<StmUartRxService,
                             config::service::stm_uart::stack_size> {
  private:
  uint8_t rx_buffer[config::stm_uart::rx_buffer_size];
  const uart_port_t port;
  std::optional<HttpQueueMessageHeader> rssp_address_to_http_queue_message_header(
      RsspAddress rssp_address);

  public:
  void main();
  StmUartRxService(int priority, uart_port_t port);
};