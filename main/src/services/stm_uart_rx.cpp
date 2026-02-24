#include "services/stm_uart_rx.hpp"
#include <cstdio>
#include "config.hpp"
#include "context.hpp"
#include "driver/uart.h"
#include "esp_log.h"
#include "esp_log_level.h"
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

    auto packet = Lappl::read_stream(self->rx_buffer);
    if (!packet.has_value()) {
      continue;
    }
    HttpQueueMessage http_queue_message;

    if (packet->header.b.resp == 1 && packet->header.b.type == LapplType::READ) {
      switch (static_cast<LapplAddress>(packet->address)) {
        case LapplAddress::RUNNING:
          http_queue_message.header = HttpQueueMessageHeader::RUNNING;
          http_queue_message.payload.b = packet->data[0];
          break;
        case LapplAddress::RIGHT_TORQUE:
          http_queue_message.header =
              HttpQueueMessageHeader::RIGHT_MANUAL_TORQUE;
          http_queue_message.payload.f = f_concat(packet->data[0],
                                                  packet->data[1],
                                                  packet->data[2],
                                                  packet->data[3]);
          break;
        case LapplAddress::LEFT_TORQUE:
          http_queue_message.header =
              HttpQueueMessageHeader::LEFT_MANUAL_TORQUE;
          http_queue_message.payload.f = f_concat(packet->data[0],
                                                  packet->data[1],
                                                  packet->data[2],
                                                  packet->data[3]);
          break;
        case LapplAddress::CONTROL_MODE:
          http_queue_message.header = HttpQueueMessageHeader::MODE;
          http_queue_message.payload.control_mode =
              static_cast<ControlMode>(packet->data[0]);
          break;
        default:
          break;
      }
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