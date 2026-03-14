#include "services/stm_uart_rx.hpp"
#include <cstdio>
#include "context/services/http.hpp"
#include "context/services/ws.hpp"
#include "driver/uart.h"
#include "freertos/idf_additions.h"
#include "hal/uart_types.h"
#include "portmacro.h"
#include "services/http.hpp"
#include "services/stm_uart/rssp.hpp"

StmUartRxService::StmUartRxService(int priority, uart_port_t port)
    : AbstractService(priority), port(port) {}

void StmUartRxService::main(StmUartRxService* self) {
  while (true) {
    int bytes_read = uart_read_bytes(self->port, self->rx_buffer, 1024, 0);
    if (bytes_read <= 0) {
      vTaskDelay(10);
      continue;
    }
    for (int i = 0; i < bytes_read; i++) {
      auto packet = Rssp::read_stream(self->rx_buffer[i]);
      if (!packet.has_value()) {
        continue;
      }
      HttpQueueMessage http_queue_message;
      if (packet->header.b.resp == 1 &&
          packet->header.b.type == RsspType::READ) {
        switch (static_cast<RsspAddress>(packet->address)) {
          case RsspAddress::RUNNING:
            http_queue_message.header = HttpQueueMessageHeader::RUNNING;
            http_queue_message.payload.b = packet->get_uint8();

            http_service.queue.send(http_queue_message, portMAX_DELAY);
            break;
          case RsspAddress::RIGHT_TORQUE:
            http_queue_message.header =
                HttpQueueMessageHeader::RIGHT_MANUAL_TORQUE;
            http_queue_message.payload.f = packet->get_float();

            http_service.queue.send(http_queue_message, portMAX_DELAY);
            break;
          case RsspAddress::LEFT_TORQUE:
            http_queue_message.header =
                HttpQueueMessageHeader::LEFT_MANUAL_TORQUE;
            http_queue_message.payload.f = packet->get_float();

            http_service.queue.send(http_queue_message, portMAX_DELAY);
            break;
          case RsspAddress::CONTROL_MODE:
            http_queue_message.header = HttpQueueMessageHeader::MODE;
            http_queue_message.payload.control_mode =
                static_cast<ControlMode>(packet->get_uint8());

            http_service.queue.send(http_queue_message, portMAX_DELAY);
            break;

          default:

            if (ws_service.has_connections()) {
              ws_service.queue.send(packet.value(), portMAX_DELAY);
            }
            
            break;
        }
      }

      else if (packet->header.b.resp == 1 &&
               packet->header.b.type == RsspType::EOC)
      {
        if (ws_service.has_connections()) {
          ws_service.queue.send(packet.value(), portMAX_DELAY);
        }
      }
    }
  }
}

void StmUartRxService::start() {
  START_SERVICE("stm_uart_rx_service");
}