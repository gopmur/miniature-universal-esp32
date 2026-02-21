#include "services/stm_uart_rx.hpp"
#include <cstdio>
#include "config.hpp"
#include "context.hpp"
#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "hal/uart_types.h"
#include "portmacro.h"

StmUartRxService::StmUartRxService(uart_port_t port) : port(port) {}

void StmUartRxService::main(StmUartRxService* self) {
  while (true) {
    int ret = uart_read_bytes(self->port,
                              self->rx_buffer,
                              config::stm_uart::packet_length,
                              portMAX_DELAY);
    if (ret > 0) {
      auto packet = self->rx_buffer;
      auto header = static_cast<UartPacketType>(packet[0]);
      switch (header) {
        case UartPacketType::GET_RUNNING:
          context::http_service.queue.send(packet[1], portMAX_DELAY);
          break;
        default:
          break;
      }
    }
  }
}

void StmUartRxService::start() {
  priority = config::service::dns::priority;
  this->thread_id = xTaskCreateStatic(
      reinterpret_cast<void (*)(void*)>(StmUartRxService::main),
      "stm_uart_rx_service",
      stack_size,
      this,
      config::service::stm_uart::priority,
      stack,
      &tcb);
}