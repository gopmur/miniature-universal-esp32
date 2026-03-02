#include "services/stm_uart_rx.hpp"
#include <cstdio>
#include "config.hpp"
#include "context.hpp"
#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "hal/uart_types.h"
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

    if (packet->header.b.resp == 1 &&
        packet->header.b.type == LapplType::READ) {
      switch (static_cast<LapplAddress>(packet->address)) {
        case LapplAddress::RUNNING:
          http_queue_message.header = HttpQueueMessageHeader::RUNNING;
          http_queue_message.payload.b = packet->get_uint8();

          context::http_service.queue.send(http_queue_message, portMAX_DELAY);
          break;
        case LapplAddress::RIGHT_TORQUE:
          http_queue_message.header =
              HttpQueueMessageHeader::RIGHT_MANUAL_TORQUE;
          http_queue_message.payload.f = packet->get_float();

          context::http_service.queue.send(http_queue_message, portMAX_DELAY);
          break;
        case LapplAddress::LEFT_TORQUE:
          http_queue_message.header =
              HttpQueueMessageHeader::LEFT_MANUAL_TORQUE;
          http_queue_message.payload.f = packet->get_float();

          context::http_service.queue.send(http_queue_message, portMAX_DELAY);
          break;
        case LapplAddress::CONTROL_MODE:
          http_queue_message.header = HttpQueueMessageHeader::MODE;
          http_queue_message.payload.control_mode =
              static_cast<ControlMode>(packet->get_uint8());

          context::http_service.queue.send(http_queue_message, portMAX_DELAY);
          break;
        default:

          if (context::ws_service.has_connections()) {
            context::ws_service.queue.send(packet.value(), portMAX_DELAY);
          }
          break;
      }
    }

    else if (packet->header.b.resp == 1 &&
             packet->header.b.type == LapplType::EOC) {
      if (context::ws_service.has_connections()) {
        context::ws_service.queue.send(packet.value(), portMAX_DELAY);
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