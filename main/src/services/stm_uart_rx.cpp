#include "services/stm_uart_rx.hpp"
#include <cstdio>
#include "config.hpp"
#include "context.hpp"
#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "hal/uart_types.h"
#include "helper.hpp"
#include "portmacro.h"
#include "services/http.hpp"
#include "services/stm_uart/lappl.hpp"
#include "services/stm_uart/packet.hpp"

StmUartRxService::StmUartRxService(uart_port_t port) : port(port) {}

void StmUartRxService::main(StmUartRxService* self) {
  while (true) {
    int ret = uart_read_bytes(self->port, &self->rx_buffer, 1, portMAX_DELAY);
    if (ret <= 0)
      continue;

    auto packet = Lappl::read(self->rx_buffer);
    if (!packet.has_value()) {
      continue;
    }
    HttpQueueMessage http_queue_message;
    auto header = static_cast<UartPacketType>(packet.value()[0]);
    switch (header) {
      case UartPacketType::GET_RUNNING:
        http_queue_message.header = HttpQueueMessageHeader::RUNNING;
        http_queue_message.payload.b = packet.value()[1];
        break;
      case UartPacketType::GET_RIGHT_MANUAL_TORQUE:
        http_queue_message.header = HttpQueueMessageHeader::RIGHT_MANUAL_TORQUE;
        http_queue_message.payload.f = f_concat(packet.value()[1],
                                                packet.value()[2],
                                                packet.value()[3],
                                                packet.value()[4]);
        break;
      case UartPacketType::GET_LEFT_MANUAL_TORQUE:
        http_queue_message.header = HttpQueueMessageHeader::LEFT_MANUAL_TORQUE;
        http_queue_message.payload.f = f_concat(packet.value()[1],
                                                packet.value()[2],
                                                packet.value()[3],
                                                packet.value()[4]);
        break;
      case UartPacketType::GET_MODE:
        http_queue_message.header = HttpQueueMessageHeader::LEFT_MANUAL_TORQUE;
        http_queue_message.payload.control_mode =
            static_cast<ControlMode>(packet.value()[1]);
        break;
      default:
        break;
    }
    context::http_service.queue.send(http_queue_message, portMAX_DELAY);
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